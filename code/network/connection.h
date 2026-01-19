#ifndef CONNECTION_H
#define CONNECTION_H

#include "copyright.h"
#include "netprotocol.h"
#include "synch.h"
#include "synchlist.h"
#include "exception.h"


#define MIN(a,b) ((a) < (b) ? (a) : (b))


class ConnectionManager;

struct ReceivedData {
    char* data;
    int length;
    int offset;
    MessageFlag flags;

    ReceivedData(const char* src, int len, MessageFlag msgFlags);
    ~ReceivedData();
};

class Connection {
    friend ConnectionManager;
public:
    Connection();
    ~Connection();

    int GetConnId() const { return connId; }
    const ConnectionKey& GetKey() const { return key; }
    ConnectionState GetState();

    bool CanSend();
    bool CanReceive();

    int QueueSend(const char* data, int length, uint8_t flags);
    int Read(char* buffer, int maxLength, MessageFlag* outFlags);
    bool HasDataAvailable();


    void InitiateClose();
    bool IsTerminated();
    void Reset();

private:
    int connId = -1;
    ConnectionKey key;
    ConnectionState state = CONN_CLOSED;
    ConnectionManager* manager = nullptr;

    uint32_t sendSeqNum = 0;
    uint32_t sendAckNum = 0;
    uint32_t recvSeqNum = 0;
    uint32_t initialSeqNum = 0;

    PendingMessage pending[MAX_PENDING_MSGS];
    int pendingCount = 0;

    SynchList* recvQueue;

    Lock* lock;
    Condition* connectCond;
    Condition* acceptCond;
    Condition* recvCond;
    Condition* sendCond;
    Condition* closeCond;

    int bufferPosition = 0;

    long long lastActivityTime = 0;
    long long connectStartTime = 0;
    int connectAttempts = 0;

    void Initialize(int id, ConnectionManager* mgr);

    void SetKey(NetworkAddress remote, uint16_t local, uint16_t remotePort);
    void SetState(ConnectionState newState);

    PendingMessage* GetFreePendingSlot();
    PendingMessage* FindPending(uint32_t seqNum);

    void AcknowledgeMessage(uint32_t seqNum);
    void EnqueueReceivedData(const char* data, int length, uint8_t flags);

    uint32_t GenerateInitialSeqNum();

    void HandleSYN(const ReliableHeader* hdr, NetworkAddress from);
    void HandleSYNACK(const ReliableHeader* hdr);
    void HandleACK(const ReliableHeader* hdr);
    void HandleDATA(const ReliableHeader* hdr, const char* payload, uint8_t flags);
    void HandleFIN(const ReliableHeader* hdr);
    void HandleFINACK(const ReliableHeader* hdr);
    void HandleRST(const ReliableHeader* hdr);

    void SendControlMessage(MessageType type, uint32_t seqNum = 0, uint32_t ackNum = 0);
    void TransmitPending(PendingMessage* msg);
    void SendACK();

    bool CheckTimeouts(long long currentTime);
};

#endif