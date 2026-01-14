// reliablepost.cc
//      Implementation of reliable message transmission
//      using EVENT-DRIVEN architecture

#include "reliablepost.h"
#include "system.h"
#include "thread.h"
#include <strings.h>

//----------------------------------------------------------------------
// ReliablePost::ReliablePost
//      Initialize reliable post office with event-driven architecture
//----------------------------------------------------------------------

ReliablePost::ReliablePost(PostOffice *po) {
    postOffice = po;
    nextSeqNum = 1;  // Start from 1 (0 reserved)
    lock = new Lock("reliable post lock");

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
// ReliablePost::SendReliable
//      Send a message reliably (NON-BLOCKING)
//      Queues message for transmission and returns immediately
//      Returns sequence number assigned to message
//----------------------------------------------------------------------

unsigned int
ReliablePost::SendReliable(PacketHeader pktHdr, MailHeader mailHdr, const char *data) {
    lock->Acquire();

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

    DEBUG('n', "SendReliable: Queued message seq %d for transmission\n", seqNum);

    TransmitPending(slot);

    lock->Release();
    return seqNum;
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
        bcopy(buffer + headerSize, data, dataLength);

        DEBUG('n', "ReceiveReliable: Got data message with seq %d from machine %d\n",
              relHdr.seqNum, inPktHdr.from);

        SendAck(inPktHdr, relHdr.mailHdr, relHdr.seqNum);

    } else {
        DEBUG('n', "ReceiveReliable: WARNING - Got ACK in MAIL_BOX (seq %d)\n",
              relHdr.seqNum);
    }
}

//----------------------------------------------------------------------
// ReliablePost::ProcessEvents
//      EVENT LOOP: Check for ACKs and handle retransmissions
//      Returns true if there are still pending messages
//----------------------------------------------------------------------

bool
ReliablePost::ProcessEvents() {
    lock->Acquire();

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
//----------------------------------------------------------------------

void
ReliablePost::WaitForPending() {
    while (ProcessEvents()) {
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

    // Prepare full message (header + data)
    char buffer[MaxMailSize];
    bcopy(&relHdr, buffer, sizeof(ReliableMailHeader));
    bcopy(msg->data, buffer + sizeof(ReliableMailHeader), msg->mailHdr.length);

    // Update mail header length to include reliable header
    MailHeader outMailHdr = msg->mailHdr;
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

        // Receive the ACK (won't block since we know there's a message)
        postOffice->Receive(ACK_BOX, &pktHdr, &mailHdr, buffer);

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
