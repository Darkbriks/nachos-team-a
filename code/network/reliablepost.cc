// reliablepost.cc
//      Implementation of reliable message transmission with TCP-like connection management
//      using EVENT-DRIVEN architecture

#include "reliablepost.h"
#include "system.h"
#include "thread.h"
#include "exception.h"
#include <strings.h>

// Special mailbox for connection control messages
#define CONTROL_BOX 2

//----------------------------------------------------------------------
// ReliablePost::ReliablePost
//      Initialize reliable post office with event-driven architecture
//----------------------------------------------------------------------

ReliablePost::ReliablePost(PostOffice *po, int remote) {
    postOffice = po;
    remoteAddr = remote;
    nextSeqNum = 1;  // Start from 1 (0 reserved)
    lock = new Lock("reliable post lock");
    connState = CONN_CLOSED;
    connectAttempts = 0;
    lastConnectTime = 0;
    peerCloseReceived = false;

    // Initialize pending messages array
    for (int i = 0; i < MAX_PENDING_MSGS; i++) {
        pendingMsgs[i].active = false;
        pendingMsgs[i].ackReceived = false;
    }
}

//----------------------------------------------------------------------
// ReliablePost::~ReliablePost
//      Clean up
//----------------------------------------------------------------------

ReliablePost::~ReliablePost() {
    delete lock;
}

//----------------------------------------------------------------------
// ReliablePost::Connect
//      Establish connection with remote machine (TCP-like 3-way handshake)
//      Blocks until connection is established or timeout
//----------------------------------------------------------------------

bool
ReliablePost::Connect() {
    printf("[Machine %d] Initiating connection to machine %d...\n",
           postOffice->GetNetAddr(), remoteAddr);
    fflush(stdout);

    lock->Acquire();
    connState = CONN_SYN_SENT;
    connectAttempts = 0;
    lastConnectTime = 0;
    lock->Release();

    // Send initial SYN
    SendControlMsg(MSG_SYN);

    // Event loop: wait for connection to be established
    while (true) {
        currentThread->Yield();

        lock->Acquire();

        // Check for incoming connection messages
        CheckForConnectionMsgs();

        // Check if connected
        if (connState == CONN_ESTABLISHED) {
            lock->Release();
            printf("[Machine %d] Connection established with machine %d!\n",
                   postOffice->GetNetAddr(), remoteAddr);
            fflush(stdout);
            return true;
        }

        // Check for timeout on SYN
        if (connState == CONN_SYN_SENT) {
            long long now = stats->totalTicks;
            if (now - lastConnectTime > CONNECT_TEMPO) {
                connectAttempts++;
                if (connectAttempts >= CONNECT_RETRIES) {
                    printf("[Machine %d] Connection FAILED: timeout after %d attempts\n",
                           postOffice->GetNetAddr(), connectAttempts);
                    fflush(stdout);
                    connState = CONN_CLOSED;
                    lock->Release();
                    return false;
                }
                // Retransmit SYN
                if (connectAttempts % 10 == 0) {
                    printf("[Machine %d] Waiting for peer... (attempt %d/%d)\n",
                           postOffice->GetNetAddr(), connectAttempts, CONNECT_RETRIES);
                    fflush(stdout);
                }
                lock->Release();
                SendControlMsg(MSG_SYN);
                lock->Acquire();
            }
        }

        lock->Release();
    }
}

//----------------------------------------------------------------------
// ReliablePost::IsConnected
//      Check if connection is established and usable for sending
//      Returns false if peer has sent CLOSE (we're in CLOSE_WAIT)
//----------------------------------------------------------------------

bool
ReliablePost::IsConnected() {
    lock->Acquire();
    bool connected = (connState == CONN_ESTABLISHED);
    lock->Release();
    return connected;
}

//----------------------------------------------------------------------
// ReliablePost::Close
//      Close connection gracefully
//----------------------------------------------------------------------

void
ReliablePost::Close() {
    lock->Acquire();

    if (connState != CONN_ESTABLISHED && connState != CONN_CLOSE_WAIT) {
        lock->Release();
        return;
    }

    // Wait for all pending messages first
    lock->Release();
    WaitForPending();
    lock->Acquire();

    if (peerCloseReceived) {
        // Peer already sent CLOSE, just send CLOSE-ACK
        printf("[Machine %d] Sending CLOSE-ACK to machine %d\n",
               postOffice->GetNetAddr(), remoteAddr);
        fflush(stdout);
        lock->Release();
        SendControlMsg(MSG_CLOSE_ACK);
        lock->Acquire();
        connState = CONN_TERMINATED;
    } else {
        // We're initiating close
        printf("[Machine %d] Initiating connection close with machine %d\n",
               postOffice->GetNetAddr(), remoteAddr);
        fflush(stdout);
        connState = CONN_CLOSING;
        lock->Release();
        SendControlMsg(MSG_CLOSE);
        lock->Acquire();

        // Wait for CLOSE-ACK
        int closeAttempts = 0;
        long long lastCloseTime = stats->totalTicks;

        while (connState == CONN_CLOSING) {
            lock->Release();
            currentThread->Yield();
            lock->Acquire();

            CheckForConnectionMsgs();

            // Timeout and retransmit CLOSE
            long long now = stats->totalTicks;
            if (now - lastCloseTime > TEMPO) {
                closeAttempts++;
                if (closeAttempts >= MAXREEMISSIONS) {
                    printf("[Machine %d] Close timeout, forcing termination\n",
                           postOffice->GetNetAddr());
                    fflush(stdout);
                    connState = CONN_TERMINATED;
                    break;
                }
                lock->Release();
                SendControlMsg(MSG_CLOSE);
                lock->Acquire();
                lastCloseTime = stats->totalTicks;
            }
        }
    }

    printf("[Machine %d] Connection terminated\n", postOffice->GetNetAddr());
    fflush(stdout);
    lock->Release();
}

//----------------------------------------------------------------------
// ReliablePost::SendControlMsg
//      Send a control message (SYN, SYN-ACK, CLOSE, CLOSE-ACK)
//----------------------------------------------------------------------

void
ReliablePost::SendControlMsg(int type) {
    PacketHeader pktHdr;
    MailHeader mailHdr;
    ReliableMailHeader ctrlHdr;

    ctrlHdr.type = type;
    ctrlHdr.seqNum = 0;  // Control messages don't use sequence numbers

    pktHdr.to = remoteAddr;
    mailHdr.to = CONTROL_BOX;
    mailHdr.from = CONTROL_BOX;
    mailHdr.length = sizeof(ReliableMailHeader);

    const char* typeStr = (type == MSG_SYN) ? "SYN" :
                          (type == MSG_SYN_ACK) ? "SYN-ACK" :
                          (type == MSG_CLOSE) ? "CLOSE" :
                          (type == MSG_CLOSE_ACK) ? "CLOSE-ACK" : "UNKNOWN";
    DEBUG('n', "SendControlMsg: Sending %s to machine %d\n", typeStr, remoteAddr);

    postOffice->Send(pktHdr, mailHdr, (char *)&ctrlHdr);

    lock->Acquire();
    if (type == MSG_SYN) {
        lastConnectTime = stats->totalTicks;
    }
    lock->Release();
}

//----------------------------------------------------------------------
// ReliablePost::CheckForConnectionMsgs
//      Check for incoming connection control messages
//----------------------------------------------------------------------

void
ReliablePost::CheckForConnectionMsgs() {
    // Must be called with lock held

    while (postOffice->HasMessages(CONTROL_BOX)) {
        char buffer[MaxMailSize];
        PacketHeader pktHdr;
        MailHeader mailHdr;

        // Temporarily release lock during receive
        lock->Release();
        postOffice->Receive(CONTROL_BOX, &pktHdr, &mailHdr, buffer);
        lock->Acquire();

        ReliableMailHeader ctrlHdr;
        bcopy(buffer, &ctrlHdr, sizeof(ReliableMailHeader));

        ProcessControlMsg(ctrlHdr.type, pktHdr.from);
    }
}

//----------------------------------------------------------------------
// ReliablePost::ProcessControlMsg
//      Process incoming control messages and update connection state
//----------------------------------------------------------------------

void
ReliablePost::ProcessControlMsg(int type, int fromAddr) {
    // Must be called with lock held

    switch (type) {
        case MSG_SYN:
            DEBUG('n', "ProcessControlMsg: Received SYN from machine %d\n", fromAddr);
            if (connState == CONN_CLOSED || connState == CONN_SYN_SENT) {
                // Send SYN-ACK
                lock->Release();
                SendControlMsg(MSG_SYN_ACK);
                lock->Acquire();

                if (connState == CONN_SYN_SENT) {
                    // Simultaneous open - both sent SYN, connection established
                    connState = CONN_ESTABLISHED;
                } else {
                    connState = CONN_SYN_RECEIVED;
                }
            } else if (connState == CONN_SYN_RECEIVED || connState == CONN_ESTABLISHED) {
                // Duplicate SYN, resend SYN-ACK
                lock->Release();
                SendControlMsg(MSG_SYN_ACK);
                lock->Acquire();
            }
            break;

        case MSG_SYN_ACK:
            DEBUG('n', "ProcessControlMsg: Received SYN-ACK from machine %d\n", fromAddr);
            if (connState == CONN_SYN_SENT || connState == CONN_SYN_RECEIVED) {
                connState = CONN_ESTABLISHED;
                lock->Release();
                SendControlMsg(MSG_SYN_ACK);
                lock->Acquire();
            }
            break;

        case MSG_CLOSE:
            DEBUG('n', "ProcessControlMsg: Received CLOSE from machine %d\n", fromAddr);
            printf("[Machine %d] Received CLOSE from peer\n", postOffice->GetNetAddr());
            fflush(stdout);
            peerCloseReceived = true;
            if (connState == CONN_ESTABLISHED) {
                connState = CONN_CLOSE_WAIT;
                // Mark all pending messages as "acked" - peer is closing, ACKs won't arrive
                // This allows IsAcked() to return true so the sender loop can exit
                for (int i = 0; i < MAX_PENDING_MSGS; i++) {
                    if (pendingMsgs[i].active && !pendingMsgs[i].ackReceived) {
                        pendingMsgs[i].ackReceived = true;  // Pretend ACKed to unblock sender
                    }
                }
            } else if (connState == CONN_CLOSING) {
                // Simultaneous close
                lock->Release();
                SendControlMsg(MSG_CLOSE_ACK);
                lock->Acquire();
                connState = CONN_TERMINATED;
            }
            break;

        case MSG_CLOSE_ACK:
            DEBUG('n', "ProcessControlMsg: Received CLOSE-ACK from machine %d\n", fromAddr);
            if (connState == CONN_CLOSING) {
                connState = CONN_TERMINATED;
            }
            break;
    }
}

//----------------------------------------------------------------------
// ReliablePost::SendReliable
//      Send a message reliably (NON-BLOCKING)
//      Queues message for transmission and returns immediately
//      Returns sequence number assigned to message, or 0 if not connected
//----------------------------------------------------------------------

unsigned int
ReliablePost::SendReliable(PacketHeader pktHdr, MailHeader mailHdr, const char *data) {
    lock->Acquire();

    // Check if connected
    if (connState != CONN_ESTABLISHED && connState != CONN_CLOSE_WAIT) {
        lock->Release();
        printf("SendReliable: ERROR - Not connected!\n");
        return 0;
    }

    // Get free slot for this message
    PendingMessage *slot = GetFreeSlot();
    if (slot == NULL) {
        lock->Release();
        printf("SendReliable: ERROR - No free slots! Too many pending messages.\n");
        return 0;
    }

    unsigned int seqNum = nextSeqNum++;

    slot->active = true;
    slot->seqNum = seqNum;
    slot->pktHdr = pktHdr;
    slot->mailHdr = mailHdr;
    bcopy(data, slot->data, mailHdr.length);
    slot->attempts = 0;
    slot->sentTime = 0;
    slot->ackReceived = false;
    slot->isChunked = false;

    DEBUG('n', "SendReliable: Queued message seq %d for transmission\n", seqNum);

    TransmitPending(slot);

    lock->Release();
    return seqNum;
}

//----------------------------------------------------------------------
// ReliablePost::SendReliableAnySize
//      Send a message of any size (the message will be divided in multiple chunks when it reachs the maximum size) reliably (NON-BLOCKING)
//      If the message length is superrior to MAX_PUT_STRING, message will be reduced to MAX_PUT_STRING size
//      Queues message for transmission and returns immediately
//      Returns sequence number assigned to message, or 0 if not connected
//----------------------------------------------------------------------

unsigned int
ReliablePost::SendReliableAnySize(PacketHeader pktHdr, MailHeader mailHdr, const char *data) {
    lock->Acquire();

    unsigned int dataLength = strlen(data);
    char *dataToSend = (char *) malloc(dataLength);
    char *small_data = (char *) malloc(MAX_PUT_STRING);
    
    if (dataLength > MAX_PUT_STRING){
        strncpy(small_data, data, MAX_PUT_STRING);
        small_data[MAX_PUT_STRING-1] = 0;
        dataLength = MAX_PUT_STRING;
    }
    else{
        strncpy(small_data,data,dataLength);
    }

    totalChunkSize = dataLength;

    // Check if connected
    if (connState != CONN_ESTABLISHED && connState != CONN_CLOSE_WAIT) {
        lock->Release();
        printf("SendReliable: ERROR - Not connected!\n");
        return 0;
    }

    
    if (sizeof(pktHdr)+sizeof(mailHdr)+dataLength >= MaxMailSize) {
        unsigned int seqNum = nextSeqNum++;
        
        ChunkHeader_t chkHdr;

        unsigned int i = 0;
        while (i < sizeof(dataToSend)) {
            
            chkHdr.isFirstChunk = false;
            chkHdr.isLastChunked = false;

            if (i == 0){
                chkHdr.isFirstChunk = true;
            }
            
            if(i+1 >= sizeof(dataToSend)){
                chkHdr.isLastChunked = true;
            }

            if (dataLength > (i+1)*MaxMailSize){
                memcpy(small_data, (dataToSend + i*MaxMailSize), MaxMailSize);
            }


            // Get free slot for this message
            PendingMessage *slot = GetFreeSlot();
            if (slot == NULL) {
                lock->Release();
                printf("SendReliableAnySize: ERROR - No free slots! Too many pending messages.\n");
                return 0;
            }

            slot->active = true;
            slot->seqNum = seqNum;
            slot->pktHdr = pktHdr;
            slot->mailHdr = mailHdr;
            bcopy(small_data, slot->data, mailHdr.length);
            slot->attempts = 0;
            slot->sentTime = 0;
            slot->ackReceived = false;
            slot->isChunked = true;
            slot->chunkedData = chkHdr;
            DEBUG('n', "SendReliableAnySize: Queued message seq %d for transmission\n", seqNum);

            TransmitPending(slot);
            i++;
        }
        lock->Release();
        return seqNum;
    }
    lock->Release();
    return SendReliable(pktHdr, mailHdr, data);
}

//----------------------------------------------------------------------
// ReliablePost::ReceiveReliable
//      Receive a message and automatically send ACK
//----------------------------------------------------------------------

void
ReliablePost::ReceiveReliable(int box, PacketHeader *pktHdr, MailHeader *mailHdr, char *data) {
    char buffer[MaxMailSize];
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    ChunkHeader_t chkHdr;

    // Receive data message (should only be called when HasMessages returns true)
    postOffice->Receive(box, &inPktHdr, &inMailHdr, buffer);

    // Parse the reliable header
    ReliableMailHeader relHdr;
    bcopy(buffer, &relHdr, sizeof(ReliableMailHeader));

    if (relHdr.type == MSG_DATA) {
        
        int headerSize = sizeof(ReliableMailHeader);
        int dataLength = inMailHdr.length - headerSize;
        
        *pktHdr = inPktHdr;
        *mailHdr = relHdr.mailHdr;  // Use original mail header from sender
        mailHdr->length = dataLength;  // Fix length to exclude reliable header
        
        if (relHdr.isChunked){
            chkHdr = relHdr.chunkedData;
            if (chkHdr.isFirstChunk){
                memcpy(rebuildBuffer, data, dataLength);
            }
            else{
                strcat((char *)rebuildBuffer, data);
            }

            if (chkHdr.isLastChunked){
                bcopy(buffer + headerSize, rebuildBuffer, totalChunkSize - headerSize);
            }
        }
        else{
            bcopy(buffer + headerSize, data, dataLength);
        }

        DEBUG('n', "ReceiveReliable: Got data message with seq %d from machine %d\n",
              relHdr.seqNum, inPktHdr.from);

        SendAck(inPktHdr, relHdr.mailHdr, relHdr.seqNum);

    } else {
        DEBUG('n', "ReceiveReliable: WARNING - Got non-data message type %d in MAIL_BOX\n",
              relHdr.type);
    }
}

//----------------------------------------------------------------------
// ReliablePost::ProcessEvents
//      EVENT LOOP: Check for ACKs, connection msgs, and handle retransmissions
//      Returns true if there are still pending messages
//----------------------------------------------------------------------

bool
ReliablePost::ProcessEvents() {
    lock->Acquire();

    // Check for incoming connection messages
    CheckForConnectionMsgs();

    // Check for incoming ACKs (non-blocking)
    CheckForAcks();

    // Check for timeouts and retransmit if needed
    CheckForTimeouts();

    int pending = 0;
    for (int i = 0; i < MAX_PENDING_MSGS; i++) {
        if (pendingMsgs[i].active && !pendingMsgs[i].ackReceived) {
            pending++;
        }
    }

    lock->Release();
    return (pending > 0);
}

//----------------------------------------------------------------------
// ReliablePost::IsAcked
//      Check if a specific message has been ACKed
//----------------------------------------------------------------------

bool
ReliablePost::IsAcked(unsigned int seqNum) {
    lock->Acquire();
    PendingMessage *msg = FindPending(seqNum);
    bool acked = (msg != NULL && msg->ackReceived);
    lock->Release();
    return acked;
}

//----------------------------------------------------------------------
// ReliablePost::WaitForPending
//      Wait for all pending messages to be ACKed or fail
//      Also exits early if peer has sent CLOSE
//----------------------------------------------------------------------

void
ReliablePost::WaitForPending() {
    while (ProcessEvents()) {
        // Check if peer sent CLOSE - no point waiting for ACKs
        lock->Acquire();
        if (peerCloseReceived || connState == CONN_CLOSE_WAIT || connState == CONN_TERMINATED) {
            lock->Release();
            break;
        }
        lock->Release();
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ReliablePost::FindPending
//      Find pending message by sequence number
//----------------------------------------------------------------------

PendingMessage*
ReliablePost::FindPending(unsigned int seqNum) {
    for (int i = 0; i < MAX_PENDING_MSGS; i++) {
        if (pendingMsgs[i].active && pendingMsgs[i].seqNum == seqNum) {
            return &pendingMsgs[i];
        }
    }
    return NULL;
}

//----------------------------------------------------------------------
// ReliablePost::GetFreeSlot
//      Get free slot for new pending message
//----------------------------------------------------------------------

PendingMessage*
ReliablePost::GetFreeSlot() {
    for (int i = 0; i < MAX_PENDING_MSGS; i++) {
        if (!pendingMsgs[i].active || pendingMsgs[i].ackReceived) {
            return &pendingMsgs[i];
        }
    }
    return NULL;
}

//----------------------------------------------------------------------
// ReliablePost::TransmitPending
//      Send or retransmit a pending message
//----------------------------------------------------------------------

void
ReliablePost::TransmitPending(PendingMessage *msg) {
    if (msg->attempts >= MAXREEMISSIONS + 1) {
        printf("SendReliable: Failed to deliver message (seq %d) after %d attempts\n",
               msg->seqNum, msg->attempts);
        msg->active = false;  // Give up
        return;
    }

    // Prepare reliable header
    ReliableMailHeader relHdr;
    relHdr.type = MSG_DATA;
    relHdr.seqNum = msg->seqNum;
    relHdr.mailHdr = msg->mailHdr;
    relHdr.isChunked = msg->isChunked;
    if (msg->isChunked){
        relHdr.chunkedData = msg->chunkedData;
    }

    // Prepare full message (header + data)
    char buffer[MaxMailSize];
    bcopy(&relHdr, buffer, sizeof(ReliableMailHeader));
    bcopy(msg->data, buffer + sizeof(ReliableMailHeader), msg->mailHdr.length);

    // Update mail header length to include reliable header
    MailHeader outMailHdr = relHdr.mailHdr;
    outMailHdr.length += sizeof(ReliableMailHeader);

    if (msg->attempts > 0) {
        printf("Retransmitting message (seq %d), attempt %d/%d\n",
               msg->seqNum, msg->attempts + 1, MAXREEMISSIONS + 1);
        fflush(stdout);
    }

    postOffice->Send(msg->pktHdr, outMailHdr, buffer);

    msg->attempts++;
    msg->sentTime = stats->totalTicks;

    DEBUG('n', "TransmitPending: Sent message seq %d (attempt %d)\n",
          msg->seqNum, msg->attempts);
}

//----------------------------------------------------------------------
// ReliablePost::ProcessAck
//      Process incoming ACK
//----------------------------------------------------------------------

void
ReliablePost::ProcessAck(unsigned int seqNum) {
    PendingMessage *msg = FindPending(seqNum);
    if (msg != NULL && !msg->ackReceived) {
        DEBUG('n', "ProcessAck: Received ACK for seq %d\n", seqNum);
        msg->ackReceived = true;
        // Keep active = true so IsAcked can still find this message!
        // Slot will be reused later by GetFreeSlot when needed
    }
}

//----------------------------------------------------------------------
// ReliablePost::SendAck
//      Send an ACK message back to sender
//----------------------------------------------------------------------

void
ReliablePost::SendAck(PacketHeader inPktHdr, MailHeader inMailHdr, unsigned int seqNum) {
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    ReliableMailHeader ackHdr;

    // Prepare ACK header
    ackHdr.type = MSG_ACK;
    ackHdr.seqNum = seqNum;
    ackHdr.mailHdr = inMailHdr;  // Include original header info

    // Set up packet header (send back to sender)
    outPktHdr.to = inPktHdr.from;

    // Set up mail header (send to ACK_BOX)
    outMailHdr.to = ACK_BOX;  // ACKs always go to ACK_BOX
    outMailHdr.from = inMailHdr.to;
    outMailHdr.length = sizeof(ReliableMailHeader);

    DEBUG('n', "SendAck: Sending ACK for seq %d to machine %d, mailbox %d\n",
          seqNum, outPktHdr.to, outMailHdr.to);

    // Send the ACK
    postOffice->Send(outPktHdr, outMailHdr, (char *)&ackHdr);
}

//----------------------------------------------------------------------
// ReliablePost::CheckForAcks
//      Check for incoming ACKs and process them (non-blocking)
//----------------------------------------------------------------------

void
ReliablePost::CheckForAcks() {
    // Check if there are ACKs waiting on ACK_BOX
    while (postOffice->HasMessages(ACK_BOX)) {
        char buffer[MaxMailSize];
        PacketHeader pktHdr;
        MailHeader mailHdr;

        // Temporarily release lock during receive
        lock->Release();
        postOffice->Receive(ACK_BOX, &pktHdr, &mailHdr, buffer);
        lock->Acquire();

        // Parse the ACK
        ReliableMailHeader ackHdr;
        bcopy(buffer, &ackHdr, sizeof(ReliableMailHeader));

        if (ackHdr.type == MSG_ACK) {
            ProcessAck(ackHdr.seqNum);
        }
    }
}

//----------------------------------------------------------------------
// ReliablePost::CheckForTimeouts
//      Check for timed-out messages and retransmit
//----------------------------------------------------------------------

void
ReliablePost::CheckForTimeouts() {
    long long now = stats->totalTicks;

    for (int i = 0; i < MAX_PENDING_MSGS; i++) {
        PendingMessage *msg = &pendingMsgs[i];

        if (msg->active && !msg->ackReceived) {
            if (now - msg->sentTime > TEMPO) {
                TransmitPending(msg);
            }
        }
    }
}
