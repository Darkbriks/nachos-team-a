// reliablepost.cc
//      Routines to provide reliable message delivery on top of
//      the unreliable network layer.
//
//      Uses ACK + timeout + retransmission mechanism (similar to TCP)

#include "reliablepost.h"
#include "system.h"
#include "interrupt.h"
#include "thread.h"
#include "process.h"

//----------------------------------------------------------------------
// ReliablePost::ReliablePost
//      Initialize a reliable post office wrapper
//
//      "po" -- pointer to the underlying unreliable PostOffice
//----------------------------------------------------------------------

ReliablePost::ReliablePost(PostOffice *po) {
    postOffice = po;
    nextSeqNum = 0;
    sendLock = new Lock("reliable send lock");

    // Initialize ACK reception infrastructure
    ackLock = new Lock("ACK lock");
    maxPendingAcks = MAX_PENDING_ACKS;
    receivedAcks = new bool[maxPendingAcks];
    for (unsigned int i = 0; i < maxPendingAcks; i++) {
        receivedAcks[i] = false;
    }
    stopAckReceiver = false;

    // Start ACK receiver thread
    Process *mainProcess = Process::FindProcessByPID(0);
    ackReceiverThread = mainProcess->CreateThread("ACK receiver");
    ackReceiverThread->Fork(AckReceiverHelper, (int)this);

    // Give thread a moment to start
    Delay(1);

    DEBUG('n', "ReliablePost: Started with ACK receiver thread\n");
}

ReliablePost::~ReliablePost() {
    // Stop ACK receiver thread
    stopAckReceiver = true;

    // Wait a bit for thread to finish (simple approach)
    Delay(1);

    delete sendLock;
    delete ackLock;
    delete[] receivedAcks;
    // Note: Don't delete postOffice - we don't own it
}


bool ReliablePost::SendReliable(PacketHeader pktHdr, MailHeader mailHdr, const char *data) {
    sendLock->Acquire();

    unsigned int seqNum = nextSeqNum+= 1; // seq nu of the mnessage
    
    // Create extended head with seq num n type 
    char buffer[MaxMailSize];

    ReliableMailHeader relHdr;
    relHdr.type = MSG_DATA;
    relHdr.seqNum = seqNum;
    relHdr.mailHdr = mailHdr;

    // Pack: ReliableMailHeader + data
    int headerSize = sizeof(ReliableMailHeader);
    bcopy(&relHdr, buffer, headerSize);
    bcopy(data, buffer + headerSize, mailHdr.length);

    // Update mail hdr length to include size of reliable header
    MailHeader extMailHdr = mailHdr;

    extMailHdr.length = headerSize + mailHdr.length;

    DEBUG('n', "SnedReliable : Sending message with seq %d to machine %d\n", seqNum, pktHdr.to);

    // Try senbding with retransmission 
    for (int attempt = 0 ; attempt < MAXREEMISSIONS ; attempt+= 1) {
       if (attempt > 0) { 
            printf ("Retransmitting message (seq %d), attempt %d/%d\n", seqNum, attempt+1, MAXREEMISSIONS+1);
            fflush(stdout);
       }
       postOffice->Send(pktHdr, extMailHdr, buffer);

       // Wait for ACK with timeout
       bool ackReceived = WaitForAckWithTimeout(mailHdr.from, seqNum, TEMPO);

       if (ackReceived) {
           DEBUG('n', "SendReliable : ACK received for (seq %d)\n", seqNum);
           sendLock->Release();
           return true;
        }

       // If we're here, timeout, will retry till success
       DEBUG('n', "SendReliable : TImeout waiting for (seq %d) attempt %d/%d\n", seqNum, attempt+1, MAXREEMISSIONS+1 );

    }

    // Failed after MAXREEMISSIONS retries
    printf("SendReliable : FGaield to deliver message (seq %d) after %d attempts\n", seqNum, MAXREEMISSIONS +1);    fflush(stdout);
    sendLock->Release();
    return false;

}
bool ReliablePost::WaitForAckWithTimeout(int ackBox, unsigned int seqNum, long long timeout) {
    long long deadline = stats->totalTicks + timeout;

    DEBUG('n', "WaitForAckWithTimeout: Waiting for ACK seq %d until tick %lld\n",seqNum, deadline);

    while (stats->totalTicks < deadline) {
        // check if ACK is available 
        if (CheckForAck(ackBox, seqNum)) {
            return true;  // ACK received!
        }

        // sleep a bit before checking again
        long long sleepDuration = POLL_INTERVAL;
        long long remaining = deadline - stats->totalTicks;
        if (remaining < sleepDuration) {
            sleepDuration = remaining;
        }

        if (sleepDuration > 0) {
            IntStatus oldLevel = interrupt->SetLevel(IntOff);
            currentThread->SleepUntil(stats->totalTicks + sleepDuration);
            interrupt->SetLevel(oldLevel);
        }
    }

    DEBUG('n', "WaitForAckWithTimeout: Timeout waiting for ACK seq %d\n", seqNum);
    return false;  // Timeout
}




void ReliablePost::SendAck(PacketHeader inPktHdr, MailHeader inMailHdr, unsigned int seqNum) {
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    ReliableMailHeader ackHdr;

    // Prepare ACK header
    ackHdr.type = MSG_ACK;
    ackHdr.seqNum = seqNum;
    ackHdr.mailHdr = inMailHdr;  // Include original header info

    // Set up packet header (send back to sender)
    outPktHdr.to = inPktHdr.from;

    // Set up mail header (send to the "from" mailbox)
    outMailHdr.to = inMailHdr.from;
    outMailHdr.from = inMailHdr.to;
    outMailHdr.length = sizeof(ReliableMailHeader);

    DEBUG('n', "SendAck: Sending ACK for seq %d to machine %d, mailbox %d\n",
          seqNum, outPktHdr.to, outMailHdr.to);

    // Send the ACK
    postOffice->Send(outPktHdr, outMailHdr, (char *)&ackHdr);
}



void ReliablePost::ReceiveReliable(int box, PacketHeader *pktHdr, MailHeader *mailHdr, char *data) {
    char buffer[MaxMailSize];
    PacketHeader inPktHdr;
    MailHeader inMailHdr;

    while (true) {
        // Receive any message (could be data or ACK)
        postOffice->Receive(box, &inPktHdr, &inMailHdr, buffer);

        // Parse the reliable header
        ReliableMailHeader relHdr;
        bcopy(buffer, &relHdr, sizeof(ReliableMailHeader));

        if (relHdr.type == MSG_DATA) {
            // This is a data message - extract payload and send ACK
            int headerSize = sizeof(ReliableMailHeader);
            int dataLength = inMailHdr.length - headerSize;

            *pktHdr = inPktHdr;
            *mailHdr = relHdr.mailHdr;  // Use original mail header from sender
            mailHdr->length = dataLength;  // Fix length to exclude reliable header
            bcopy(buffer + headerSize, data, dataLength);

            DEBUG('n', "ReceiveReliable: Got data message with seq %d from machine %d\n",
                  relHdr.seqNum, inPktHdr.from);

            // Send ACK back
            SendAck(inPktHdr, relHdr.mailHdr, relHdr.seqNum);

            return; 

        } else if (relHdr.type == MSG_ACK) {
            // This is an ACK - it will be picked up by WaitForAckWithTimeout
            // Just ignore it here (it's already in the mailbox history)
            DEBUG('n', "ReceiveReliable: Got ACK with seq %d (ignoring in data receive)\n",
                  relHdr.seqNum);
            continue;  // Keep waiting for a data message
        }
    }
}


bool ReliablePost::CheckForAck(int ackBox, unsigned int expectedSeqNum)
{
    ackLock->Acquire();
    unsigned int index = expectedSeqNum % maxPendingAcks;
    bool found = receivedAcks[index];
    if (found) {
        receivedAcks[index] = false;  // Clear after reading
        DEBUG('n', "CheckForAck: Found ACK for seq %d\n", expectedSeqNum);
    }
    ackLock->Release();
    return found;
}

void ReliablePost::MarkAckReceived(unsigned int seqNum)
{
    ackLock->Acquire();
    unsigned int index = seqNum % maxPendingAcks;
    receivedAcks[index] = true;
    DEBUG('n', "MarkAckReceived: Marked ACK for seq %d\n", seqNum);
    ackLock->Release();
}

void ReliablePost::AckReceiverHelper(int arg)
{
    ReliablePost *reliablePost = (ReliablePost *)arg;
    reliablePost->AckReceiverLoop();
}

void ReliablePost::AckReceiverLoop()
{
    char buffer[MaxMailSize];
    PacketHeader pktHdr;
    MailHeader mailHdr;

    DEBUG('n', "AckReceiverLoop: ACK receiver thread started\n");

    while (!stopAckReceiver) {
        // Receive from mailbox 1 (where ACKs are sent)
        // This will block until a message arrives
        postOffice->Receive(1, &pktHdr, &mailHdr, buffer);

        // Parse the reliable header
        ReliableMailHeader relHdr;
        bcopy(buffer, &relHdr, sizeof(ReliableMailHeader));

        if (relHdr.type == MSG_ACK) {
            // This is an ACK - mark it as received
            DEBUG('n', "AckReceiverLoop: Received ACK for seq %d from machine %d\n",
                  relHdr.seqNum, pktHdr.from);
            MarkAckReceived(relHdr.seqNum);
        } else {
            // This is not an ACK (shouldn't happen on mailbox 1)
            DEBUG('n', "AckReceiverLoop: WARNING - received non-ACK message on ACK mailbox\n");
        }
    }

    DEBUG('n', "AckReceiverLoop: ACK receiver thread stopping\n");
}
