#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include "copyright.h"
#include "netprotocol.h"
#include "connection.h"
#include "synch.h"

struct Listener {
    bool active = false;
    uint16_t port = 0;
    Lock* lock;
    Condition* acceptCond;

    Connection* pendingConnections[MAX_PENDING_ACCEPTS];
    int pendingCount = 0;

    Listener();
    ~Listener();
    void Reset();
};

class ConnectionManager {
    friend Connection;

public:
    static ConnectionManager* GetInstance();

    void Initialize(PostOffice* po);
    void Shutdown();

    NetworkAddress GetLocalAddress();
    ConnectionState GetConnectionState(int connId);

    int Connect(NetworkAddress remoteAddr, uint16_t remotePort, uint16_t localPort = PORT_AUTO);
    int Listen(uint16_t port);
    int Accept(int listenerId, int timeoutMs = DEFAULT_ACCEPT_TIMEOUT);
    int CloseListener(int listenerId);

    int Send(int connId, const char* data, int length);
    int Recv(int connId, char* buffer, int maxLength);
    int Close(int connId);

    bool IsValidConnection(int connId);

    void PrintConnections();

private:
    static ConnectionManager* instance;

    ConnectionManager();
    ~ConnectionManager();

    PostOffice* postOffice = nullptr;
    bool initialized = false;
    bool shutdownRequested = false;

    Connection* connections[MAX_CONNECTIONS];
    Lock* connectionsLock;

    Listener* listeners[MAX_LISTENERS];
    Lock* listenersLock;

    uint16_t nextEphemeralPort = PORT_RANGE_EPHEMERAL_MIN;
    Lock* portLock;

    Thread* workerThread = nullptr;


    int AllocateConnection();
    void FreeConnection(int connId);

    Connection* GetConnection(int connId);

    Connection* FindConnectionByKey(const ConnectionKey& key);
    Connection* FindConnectionByLocalPort(uint16_t localPort);

    int AllocateListener(uint16_t port);
    void FreeListener(int listenerId);
    Listener* GetListener(int listenerId);
    Listener* FindListenerByPort(uint16_t port);

    uint16_t AllocateEphemeralPort();
    bool IsPortInUse(uint16_t port)const;

    void ProcessIncomingPacket(PacketHeader pktHdr, MailHeader mailHdr, const char* data);
    void HandleIncomingSYN(PacketHeader pktHdr, const ReliableHeader* relHdr);
    void RouteToConnection(PacketHeader pktHdr, const ReliableHeader* relHdr, const char* payload);

    void SendPacket(NetworkAddress destAddr, uint16_t srcPort, uint16_t dstPort,
                   MessageType type, uint32_t seqNum, uint32_t ackNum, const char* data, int dataLen);

    void SendRST(NetworkAddress destAddr, uint16_t srcPort, uint16_t dstPort, uint32_t seqNum, uint32_t ackNum);

    void CheckAllTimeouts();

    static void WorkerThreadEntry(int arg);
    void WorkerLoop();
};

inline ConnectionManager* GetConnectionManager() {
    return ConnectionManager::GetInstance();
}

#endif