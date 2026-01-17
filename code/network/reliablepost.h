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

#if false

#include "post.h"
#include "network.h"
#include "exception.h"

// Timeout and retransmission parameters
#define TEMPO 10000              // Timeout in ticks for retransmission
#define MAXREEMISSIONS 100       // Maximum retransmission attempts
#define MAX_PENDING_MSGS 10      // Maximum pending messages
#define CONNECT_TEMPO 500000     // Longer timeout for connection establishment (10x more)
#define CONNECT_RETRIES 1000     // More retries for connection (allows ~60s manual launch)

// Message types (TCP-like)
#define MSG_DATA 1              // Regular data message
#define MSG_ACK 2               // Acknowledgment message
#define MSG_SYN 3               // Connection request (SYNchronize)
#define MSG_SYN_ACK 4           // Connection request acknowledgment
#define MSG_CLOSE 5             // Connection close request
#define MSG_CLOSE_ACK 6         // Connection close acknowledgment

// Size const
#define MaxMailSize (MaxPacketSize - sizeof(MailHeader))

// Connection states
enum ConnectionState {
    CONN_CLOSED,                // No connection
    CONN_SYN_SENT,              // SYN sent, waiting for SYN-ACK
    CONN_SYN_RECEIVED,          // SYN received, SYN-ACK sent
    CONN_ESTABLISHED,           // Connection established, can send data
    CONN_CLOSE_WAIT,            // Received CLOSE, waiting to finish sending
    CONN_CLOSING,               // Sent CLOSE, waiting for CLOSE-ACK
    CONN_TERMINATED             // Connection fully closed
};

// Structure to store chunk data
typedef struct ChunkHeader {
    bool isFirstChunk;          // Tell if this is the first chunk
    bool isLastChunked;         // Tell if the message is the last chunk
} ChunkHeader_t;

// ReliableMailHeader extends MailHeader with sequence number and message type
class ReliableMailHeader {
  public:
    unsigned char type;         // Type of the message
    unsigned int seqNum;        // Sequence number for ordering/deduplication
    MailHeader mailHdr;         // Original mail header
    bool isChunked;             // Tell if the message need to be rebuilt on receive
    bool isLastChunked;         // Tell if the message is the last chunk
    ChunkHeader_t chunkedData;    // Data of the chunk 

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
    bool isChunked;             // Tell if the message need to be rebuilt on receive
    ChunkHeader_t chunkedData;   // Data of the chunk 
};

// ReliablePost provides reliable message transmission with TCP-like connection management
// EVENT-DRIVEN: Uses non-blocking event loop instead of blocking waits
class ReliablePost {
  public:
    ReliablePost(PostOffice *po, int remoteAddr);
    ~ReliablePost();

    // CONNECTION MANAGEMENT (TCP-like handshake)
    // Connect to remote machine (blocks until connection established)
    bool Connect();

    // Check if connection is established
    bool IsConnected();

    // Close connection gracefully
    void Close();

    // Send a message reliably (NON-BLOCKING - queues for transmission)
    // Returns sequence number assigned to message, or 0 if not connected
    unsigned int SendReliable(PacketHeader pktHdr, MailHeader mailHdr, const char *data);

    // Receive a message and automatically send ACK
    void ReceiveReliable(int box, PacketHeader *pktHdr, MailHeader *mailHdr, char *data);

    // Send a message of any size (the message will be divided in multiple chunks when it reachs the maximum size) reliably (NON-BLOCKING - queues for transmission)
    // Returns sequence number assigned to message, or 0 if not connected
    unsigned int SendReliableAnySize(PacketHeader pktHdr, MailHeader mailHdr, const char *data);

    // EVENT LOOP: Process network events (ACKs, retransmissions, connection msgs)
    // Returns true if there are still pending messages or connection in progress
    bool ProcessEvents();

    // Check if a specific message has been ACKed
    bool IsAcked(unsigned int seqNum);

    // Wait for all pending messages to be ACKed or timeout
    void WaitForPending();

  private:
    PostOffice *postOffice;                   // Underlying unreliable post office
    unsigned int nextSeqNum;                  // Next sequence number to use
    Lock *lock;                               // Protect pending messages array
    int remoteAddr;                           // Address of remote machine
    ConnectionState connState;                // Current connection state
    int connectAttempts;                      // Number of SYN attempts made
    long long lastConnectTime;                // Time of last SYN sent
    bool peerCloseReceived;                   // True if peer sent CLOSE
    char *rebuildBuffer[MAX_PUT_STRING];      // Buffer to rebuild chunked messages
    int totalChunkSize;                       // Keep in memory the full size of the chunk

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

    // CONNECTION HELPERS
    // Send a control message (SYN, SYN-ACK, CLOSE, CLOSE-ACK)
    void SendControlMsg(int type);

    // Process incoming control messages
    void ProcessControlMsg(int type, int fromAddr);

    // Check for incoming connection messages
    void CheckForConnectionMsgs();
};

#endif

#endif // RELIABLEPOST_H
