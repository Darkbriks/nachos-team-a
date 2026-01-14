// reliablepost.h
//      Data structures for providing reliable message delivery
//      on top of the unreliable network layer.
//
//      Implements ACK-based reliable transmission with timeout and retransmission.
//      Similar to TCP/IP on the Internet.

#ifndef RELIABLEPOST_H
#define RELIABLEPOST_H

#include "post.h"
#include "network.h"
#include "process.h"

// Timeout and retransmission parameters
#define TEMPO 10000              // Increased for testing TODO : Modifiy later for a smaller value
#define MAXREEMISSIONS 5
#define POLL_INTERVAL 100        // Increased polling interval (was 50)
#define MAX_PENDING_ACKS 100     // Maximum number of pending ACKs to track

// Special message types
#define MSG_DATA 1              // Regular data message
#define MSG_ACK 2               // Acknowledgment message

// ReliableMailHeader extends MailHeader with sequence number and message type
class ReliableMailHeader {
  public:
    unsigned char type;
    unsigned int seqNum;        // Sequence number for ordering/deduplication
    MailHeader mailHdr;         // Original mail header

    ReliableMailHeader() : type(MSG_DATA), seqNum(0) {}
};

// ReliablePost provides reliable message transmission
// Uses ACK + timeout + retransmission on top of unreliable PostOffice
class ReliablePost {
  public:
    ReliablePost(PostOffice *po);
    ~ReliablePost();

    // Send a message reliably with ACK/timeout/retransmission
    // Returns true if message was delivered, false if all retries failed
    bool SendReliable(PacketHeader pktHdr, MailHeader mailHdr, const char *data);

    // Receive a message and automatically send ACK
    void ReceiveReliable(int box, PacketHeader *pktHdr, MailHeader *mailHdr, char *data);

  private:
    PostOffice *postOffice;     // Underlying unreliable post office
    unsigned int nextSeqNum;    // Next sequence number to use
    Lock *sendLock;             // Ensure only one send at a time

    // MAIL management
    SynchList *pendingMails;   // Pennding mails to send
    bool receivedMessages[MAX_THREAD];
    Thread *mailManagerThread; // Background thread to receive and send mails

    // ACK reception infrastructure
    Lock *ackLock;                  // Protect ACK map
    bool receivedAcks[MAX_THREAD];  // Array of received ACK flags (indexed by seqNum % MAX_PENDING)
    unsigned int maxPendingAcks;    // Maximum number of pending ACKs to track
    Thread *ackManagerThread;       // Background thread to receive and send ACKs



    void Run();

    void SendAck(PacketHeader inPktHdr, MailHeader inMailHdr, unsigned int seqNum);

    void AddPendingMessage(Mail mail);

    void RemovePendingMessage();

    void MailHandler(PacketHeader pktHdr, MailHeader mailHdr, const char *data);

    static void MailHandlerHelper(int arg);

    void AckHandler(PacketHeader pktHdr, MailHeader mailHdr, const char *data);

    static void AckHandlerHelper(int arg);
};

#endif // RELIABLEPOST_H



