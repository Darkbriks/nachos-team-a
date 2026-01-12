// reliablepost.h
//      Data structures for providing reliable message delivery
//      on top of the unreliable network layer.
//
//      Implements ACK-based reliable transmission with timeout and retransmission.
//      Similar to TCP/IP on the Internet.

#ifndef RELIABLEPOST_H
#define RELIABLEPOST_H

#include "copyright.h"
#include "post.h"
#include "network.h"

// Timeout and retransmission parameters
#define TEMPO 2000
#define MAXREEMISSIONS 5
#define POLL_INTERVAL 50

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

    // Helper: Check if ACK is available without blocking
    bool CheckForAck(int ackBox, unsigned int expectedSeqNum);

    // Helper: Wait for ACK with timeout
    bool WaitForAckWithTimeout(int ackBox, unsigned int seqNum, long long timeout);

    // Helper: Send an ACK message
    void SendAck(PacketHeader inPktHdr, MailHeader inMailHdr, unsigned int seqNum);
};

#endif // RELIABLEPOST_H



