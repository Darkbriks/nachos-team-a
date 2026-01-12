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

    // ACK reception infrastructure
    Lock *ackLock;              // Protect ACK map
    bool *receivedAcks;         // Array of received ACK flags (indexed by seqNum % MAX_PENDING)
    unsigned int maxPendingAcks;// Maximum number of pending ACKs to track
    bool stopAckReceiver;       // Signal to stop ACK received from thread
    Thread *ackReceiverThread;  // Background thread for reciving ACHs

    // Helper: Check if ACK is available without blocking
    bool CheckForAck(int ackBox, unsigned int expectedSeqNum);

    // Helper: Wait for ACK with timeout
    bool WaitForAckWithTimeout(int ackBox, unsigned int seqNum, long long timeout);

    // Helper: Send an ACK message
    void SendAck(PacketHeader inPktHdr, MailHeader inMailHdr, unsigned int seqNum);

    // Helper: ACK receiver thread function (static wrapper)
    static void AckReceiverHelper(int arg);

    // Helper: ACK receiver loop
    void AckReceiverLoop();

    // Helper: Mark ACK as received
    void MarkAckReceived(unsigned int seqNum);
};

#endif // RELIABLEPOST_H



