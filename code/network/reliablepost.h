// reliablepost.h
//      Data structures for providing reliable message delivery
//      on top of the unreliable network layer.
//
//      EVENT-DRIVEN ARCHITECTURE:
//      - No blocking waits for ACKs
//      - Main event loop checks for incoming ACKs and messages
//      - Retransmissions handled in event loop
//
//      Implements ACK-based reliable transmission with timeout and retransmission.
//      Similar to TCP/IP on the Internet.

#ifndef RELIABLEPOST_H
#define RELIABLEPOST_H

#include "post.h"
#include "network.h"

// Timeout and retransmission parameters
#define TEMPO 10000              // Timeout in ticks
#define MAXREEMISSIONS 5         // Maximum retransmission attempts
#define MAX_PENDING_MSGS 10      // Maximum pending messages

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

// Structure to track pending messages awaiting ACK
struct PendingMessage {
    bool active;                // Is this slot in use?
    unsigned int seqNum;        // Sequence number
    PacketHeader pktHdr;        // Packet header
    MailHeader mailHdr;         // Mail header
    char data[MaxMailSize];     // Message data
    int attempts;               // Number of transmission attempts
    long long sentTime;         // When was it last sent (for timeout)
    bool ackReceived;           // Has ACK been received?
};

// ReliablePost provides reliable message transmission
// EVENT-DRIVEN: Uses non-blocking event loop instead of blocking waits
class ReliablePost {
  public:
    ReliablePost(PostOffice *po);
    ~ReliablePost();

    // Send a message reliably (NON-BLOCKING - queues for transmission)
    // Returns sequence number assigned to message
    unsigned int SendReliable(PacketHeader pktHdr, MailHeader mailHdr, const char *data);

    // Receive a message and automatically send ACK
    void ReceiveReliable(int box, PacketHeader *pktHdr, MailHeader *mailHdr, char *data);

    // EVENT LOOP: Process network events (ACKs, retransmissions)
    // Returns true if there are still pending messages
    bool ProcessEvents();

    // Check if a specific message has been ACKed
    bool IsAcked(unsigned int seqNum);

    // Wait for all pending messages to be ACKed or timeout
    void WaitForPending();

  private:
    PostOffice *postOffice;     // Underlying unreliable post office
    unsigned int nextSeqNum;    // Next sequence number to use
    Lock *lock;                 // Protect pending messages array

    // Pending messages awaiting ACK
    PendingMessage pendingMsgs[MAX_PENDING_MSGS];

    // Helper: Find pending message by sequence number
    PendingMessage* FindPending(unsigned int seqNum);

    // Helper: Get free slot for new pending message
    PendingMessage* GetFreeSlot();

    // Helper: Send or retransmit a pending message
    void TransmitPending(PendingMessage *msg);

    // Helper: Process incoming ACK
    void ProcessAck(unsigned int seqNum);

    // Helper: Send an ACK message
    void SendAck(PacketHeader inPktHdr, MailHeader inMailHdr, unsigned int seqNum);

    // Helper: Check and process ACKs if available (non-blocking)
    void CheckForAcks();

    // Helper: Check and retransmit timed-out messages
    void CheckForTimeouts();
};

#endif // RELIABLEPOST_H
