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
typedef struct SendParameters{
      PacketHeader pktHdrMail;
      MailHeader mailHdrMail;
      const char *dataMail;
      PostOffice *postOfficeMail;
    } SendParameters_t;

SendParameters_t *sendMailParameters = new SendParameters_t;
SendParameters_t *sendAckParameters = new SendParameters_t;


ReliablePost::ReliablePost(PostOffice *po) {
    postOffice = po;
    nextSeqNum = 0;
    sendLock = new Lock("reliable send lock");
    

    // Initialize ACK reception infrastructure
    ackLock = new Lock("ACK lock");
    memset(receivedAcks, false, MAX_THREAD);

    // Initialize ACK reception infrastructure
    mailLock = new Lock("MAIL lock");
    memset(receivedMails, NULL, MAX_THREAD);

    pendingMails = new SynchList();

    // Threads creation
    Process * mainProcess = currentThread-> getProcess();
    ackManagerThread = mainProcess->CreateThread("ACK Manager");
    mailManagerThread = mainProcess->CreateThread("MAIL Manager");


    // Give thread a moment to start
    Delay(1);

    DEBUG('n', "ReliablePost: Started with ACK receiver thread\n");
}

ReliablePost::~ReliablePost() {
    // Wait a bit for thread to finish (simple approach)
    Delay(1);

    delete sendLock;
    delete ackLock;
    delete[] receivedAcks;
    delete mailLock;
    delete[] receivedMails;
    delete pendingMails;
    delete ackManagerThread;
    delete mailManagerThread;
    // Note: Don't delete postOffice - we don't own it
}


void ReliablePost::Run(){
    for(;;){
        //Thread ACK
        if (postOffice->AckMailBoxHasMessage()){
            AckHandler();

        }
        //Thread MAIL
        if (postOffice->MailMailBoxHasMessage()){
            MailHandler();

        }
        //Thread Main
        if (pendingMails->GetSize() > 0){
            
            Mail *firstMail = (Mail *)pendingMails->GetFirst();

            PacketHeader pktHdr = firstMail->pktHdr;
            MailHeader mailHdr = firstMail->mailHdr;
            char * data = firstMail->data;

            //Send the user's message
            bool messageSent = SendReliable(pktHdr,mailHdr,data);
            if (!messageSent){
                currentThread->SleepUntil(TEMPO);
            }

        }
    }
}

void ReliablePost::AddPendingMessage(Mail *mail){
    pendingMails->Append((void *)mail);
}

void ReliablePost::RemovePendingMessage(){
    pendingMails->Remove();
}


bool ReliablePost::SendReliable(PacketHeader pktHdr, MailHeader mailHdr, const char *data) {
    sendLock->Acquire();

    unsigned int seqNum = nextSeqNum+= 1; // seq num of the message
    
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

    DEBUG('n', "SendReliable : Sending message with seq %d to machine %d\n", seqNum, pktHdr.to);


    postOffice->Send(pktHdr, extMailHdr, buffer);
    currentThread->SleepUntil(POLL_INTERVAL);
    sendLock->Release();

    return receivedMails[relHdr.seqNum] != NULL;

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

        mailLock->Acquire();
        receivedMails[relHdr.seqNum] = new Mail(inPktHdr, relHdr.mailHdr, data);
        mailLock->Release();

        SendAck(inPktHdr, relHdr.mailHdr,relHdr.seqNum);

        DEBUG('n', "ReceiveReliable: Got data message with seq %d from machine %d\n",
              relHdr.seqNum, inPktHdr.from);

        return; 
    } else if (relHdr.type == MSG_ACK) {
        ackLock->Acquire();
        receivedAcks[relHdr.seqNum] = true;
        ackLock->Release();
        // This is an ACK - it will be picked up by WaitForAckWithTimeout
        // Just ignore it here (it's already in the mailbox history)
        DEBUG('n', "ReceiveReliable: Got ACK with seq %d (ignoring in data receive)\n",
              relHdr.seqNum);
    }
}


void ReliablePost::MailHandler(PacketHeader pktHdr, MailHeader mailHdr, char *data){
    //Receive pending MAIL
    ReceiveReliable(MAIL_BOX,&pktHdr, &mailHdr, data);

}


void ReliablePost::AckHandler(PacketHeader pktHdr, MailHeader mailHdr, char *data){
    //Receive pending ACK
    ReceiveReliable(ACK_BOX,&pktHdr, &mailHdr, data);

    //Remove message from list
    RemovePendingMessage();
    
}



