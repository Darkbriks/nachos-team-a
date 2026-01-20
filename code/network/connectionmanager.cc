#include "connectionmanager.h"
#include "system.h"
#include "thread.h"
#include "process.h"
#include "nos_errno.h"
#include "../libs/stdlib/nos_errno.h"
#include "../threads/utility.h"
#include "../userprog/syscall.h"

#include <cstdio>
#include <strings.h>


ConnectionManager* ConnectionManager::instance = nullptr;

Listener::Listener() : lock(new Lock("listener lock")), acceptCond(new Condition("accept condition")) {
    for (int i = 0; i < MAX_PENDING_ACCEPTS; i++) { pendingConnections[i] = nullptr; }
}

Listener::~Listener() {
    delete lock;
    delete acceptCond;
}

void Listener::Reset() {
    active = false;
    port = 0;
    pendingCount = 0;
    for (int i = 0; i < MAX_PENDING_ACCEPTS; i++) { pendingConnections[i] = nullptr; }
}

ConnectionManager* ConnectionManager::GetInstance() {
    if (instance == nullptr) { instance = new ConnectionManager(); }
    return instance;
}

ConnectionManager::ConnectionManager() : connectionsLock(new Lock("connections lock")),
                                         listenersLock(new Lock("listeners lock")),
                                         portLock(new Lock("port lock")) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) { connections[i] = nullptr; }
    for (int i = 0; i < MAX_LISTENERS; i++) { listeners[i] = nullptr; }
}

ConnectionManager::~ConnectionManager() {
    Shutdown();

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i] != nullptr) { delete connections[i]; }
    }

    for (int i = 0; i < MAX_LISTENERS; i++) {
        if (listeners[i] != nullptr) { delete listeners[i]; }
    }

    delete connectionsLock;
    delete listenersLock;
    delete portLock;
}

void ConnectionManager::Initialize(PostOffice* po) {
    if (initialized) { return; }

    postOffice = po;
    initialized = true;
    shutdownRequested = false;

    Process* mainProcess = Process::FindProcessByPID(0);
    workerThread = mainProcess->CreateThread("connection worker");
    workerThread->Fork(WorkerThreadEntry, (int)this);

    DEBUG('n', "ConnectionManager initialized\n");
}

void ConnectionManager::Shutdown() {
    if (!initialized) { return; }

    shutdownRequested = true;

    initialized = false;
    DEBUG('n', "ConnectionManager shutdown\n");
}

NetworkAddress ConnectionManager::GetLocalAddress() {
    return postOffice->GetNetAddr();
}

ConnectionState ConnectionManager::GetConnectionState(const int connId) {
    Connection* conn = GetConnection(connId);
    if (conn == nullptr) { return CONN_CLOSED; }
    return conn->GetState();
}

int ConnectionManager::Connect(NetworkAddress remoteAddr, uint16_t remotePort,
                               uint16_t localPort) {
    if (!initialized) { return E_INVAL; }
    if (remotePort == 0) { return E_INVAL; }

    if (localPort == PORT_AUTO) {
        localPort = AllocateEphemeralPort();
        if (localPort == 0) { return E_NOPORT; }
    } else if (IsPortInUse(localPort)) { return E_ADDRINUSE; }

    int connId = AllocateConnection();
    if (connId < 0) { return E_NOTCONN; }

    Connection* conn = GetConnection(connId);
    if (conn == nullptr) { return E_NOTCONN; }

    conn->lock->Acquire();
    conn->SetKey(remoteAddr, localPort, remotePort);
    conn->initialSeqNum = conn->GenerateInitialSeqNum();
    conn->sendSeqNum = conn->initialSeqNum;
    conn->SetState(CONN_SYN_SENT);
    conn->connectStartTime = stats->totalTicks;
    conn->connectAttempts = 0;

    conn->SendControlMessage(MessageType::MSG_SYN, conn->initialSeqNum, 0);

    while (conn->state == CONN_SYN_SENT || conn->state == CONN_SYN_RECEIVED) {
        conn->connectCond->Wait(conn->lock);
    }

    const ConnectionState finalState = conn->state;
    conn->lock->Release();

    if (finalState == CONN_ESTABLISHED) {
        DEBUG('n', "Connect: Connection %d established to %d:%d\n", connId, remoteAddr, remotePort);
        return connId;
    }

    FreeConnection(connId);
    if (finalState == CONN_TERMINATED) { return E_REFUSED; }
    return E_TIMEOUT;
}

int ConnectionManager::Listen(const uint16_t port) {
    if (!initialized) { return E_INVAL; }
    if (port == 0) { return E_INVAL; }
    if (FindConnectionByLocalPort(port) != nullptr) { return E_ADDRINUSE; }
    return AllocateListener(port);
}

int ConnectionManager::Accept(const int listenerId, int timeoutMs) {
    if (!initialized) { return E_INVAL; }

    Listener* listener = GetListener(listenerId);
    if (listener == nullptr || !listener->active) {
        return E_INVAL;
    }

    listener->lock->Acquire();

    long long wakeTime = -1;
    if (timeoutMs > 0) {
        wakeTime = stats->totalTicks + timeoutMs;
    } else if (timeoutMs == 0) {
        if (listener->pendingCount == 0) {
            listener->lock->Release();
            return E_WOULDBLOCK;
        }
    }

    while (listener->pendingCount == 0 && listener->active) {
        if (timeoutMs == 0) {
            listener->lock->Release();
            return E_WOULDBLOCK;
        }

        if (timeoutMs > 0 && stats->totalTicks >= wakeTime) {
            listener->lock->Release();
            return E_TIMEOUT;
        }

        listener->acceptCond->Wait(listener->lock);
    }

    if (!listener->active) {
        listener->lock->Release();
        return E_CLOSED;
    }

    Connection* conn = nullptr;
    for (int i = 0; i < MAX_PENDING_ACCEPTS; i++) {
        if (listener->pendingConnections[i] != nullptr) {
            conn = listener->pendingConnections[i];
            listener->pendingConnections[i] = nullptr;
            listener->pendingCount--;
            break;
        }
    }

    listener->lock->Release();

    if (conn == nullptr) {
        return E_INVAL;
    }

    conn->lock->Acquire();
    while (conn->state == CONN_SYN_RECEIVED) {
        conn->acceptCond->Wait(conn->lock);
    }

    int connId = conn->connId;
    const ConnectionState finalState = conn->state;
    conn->lock->Release();

    if (finalState == CONN_ESTABLISHED) {
        DEBUG('n', "Accept: Connection %d accepted on port %d\n", connId, listener->port);
        return connId;
    }

    FreeConnection(connId);
    return E_REFUSED;
}

int ConnectionManager::CloseListener(const int listenerId) {
    Listener* listener = GetListener(listenerId);
    if (listener == nullptr) { return E_INVAL; }

    listener->lock->Acquire();
    listener->active = false;
    listener->acceptCond->Broadcast(listener->lock);
    listener->lock->Release();

    FreeListener(listenerId);
    return E_SUCCESS;
}

int ConnectionManager::Send(const int connId, const char* data, int length) {
    if (!initialized) { return E_INVAL; }

    Connection* conn = GetConnection(connId);
    if (conn == nullptr) { return E_INVAL; }
    if (!conn->CanSend()) { return E_NOTCONN; }

    if (length == -1){
        conn->SendControlMessage(MessageType::MSG_CHUNK_BEGIN);
        return 0;
        
    } 
    else if (length == -2){
        conn->SendControlMessage(MessageType::MSG_CHUNK_END);
        return 0;
    }

    if (length > static_cast<int>(MAX_RELIABLE_DATA)) {
        if (length > MAX_PUT_STRING) { return E_INVAL; }
        DEBUG('n', "Send: Fragmenting message of length %d on connection %d\n", length, connId);
        int totalSent = 0;
        while (totalSent < length) {
            int chunkSize = MIN(static_cast<int>(MAX_RELIABLE_DATA), length - totalSent);
            uint8_t flags = static_cast<uint8_t>(MessageFlag::FLAG_MORE_FRAGMENTS);
            if (totalSent + chunkSize >= length) {
                flags = static_cast<uint8_t>(MessageFlag::FLAG_END_OF_MESSAGE);
                DEBUG('n', "Send: Sending final fragment of size %d on connection %d\n", chunkSize, connId);
            }
            if (const int result = conn->QueueSend(data + totalSent, chunkSize, flags); result < 0) {
                return result;
            }
            totalSent += chunkSize;
            DEBUG('n', "Send: Sent fragment of size %d on connection %d\n", chunkSize, connId);
        }
    } else {
        if (const int result = conn->QueueSend(data, length, static_cast<uint8_t>(MessageFlag::FLAG_END_OF_MESSAGE)); result < 0) {
            return result;
        }
        DEBUG('n', "Send: Sent message of size %d on connection %d\n", length, connId);
    }

    DEBUG('n', "Send: Total %d bytes sent on connection %d\n", length, connId);

    return length;
}

// TODO: UDP like
int ConnectionManager::Recv(const int connId, char* recv_buffer, const int maxLength, MessageType* messageType) {
    if (!initialized) { return E_INVAL; }
    Connection* conn = GetConnection(connId);
    if (conn == nullptr) { return E_INVAL; }

    int totalReceived = 0;
    char buffer[MAX_RELIABLE_DATA];
    bool endOfMessage = false;
    while (!endOfMessage && totalReceived < maxLength) {
        DEBUG('n', "Recv: Waiting for data on connection %d\n", connId);
        MessageFlag flags;
        const int bytesRead = conn->Read(buffer, MAX_RELIABLE_DATA, &flags, messageType);
        if (bytesRead < 0) {
            DEBUG('n', "Recv: Error %d reading data on connection %d\n", bytesRead, connId);
            return bytesRead;
        }
        DEBUG('n', "Recv: Received %d bytes on connection %d\n", bytesRead, connId);

        const int bytesToCopy = MIN(bytesRead, maxLength - totalReceived);
        bcopy(buffer, recv_buffer + totalReceived, bytesToCopy);
        totalReceived += bytesToCopy;
        DEBUG('n', "Recv: Copied %d bytes to user buffer on connection %d\n", bytesToCopy, connId);

        endOfMessage = flags == MessageFlag::FLAG_END_OF_MESSAGE;
        DEBUG('n', "Recv: endOfMessage=%s on connection %d\n", endOfMessage ? "true" : "false", connId);
    }

    DEBUG('n', "Recv: Total %d bytes received on connection %d\n", totalReceived, connId);
    return totalReceived;
}

int ConnectionManager::Close(const int connId) {
    if (!initialized) { return E_INVAL; }

    Connection* conn = GetConnection(connId);
    if (conn == nullptr) { return E_INVAL; }

    conn->InitiateClose();

    conn->lock->Acquire();
    long long closeTimeout = stats->totalTicks + CONNECT_TIMEOUT;

    while (conn->state != CONN_TERMINATED && conn->state != CONN_CLOSED && conn->state != CONN_TIME_WAIT) {
        if (stats->totalTicks >= closeTimeout) {
            conn->SetState(CONN_TERMINATED);
            break;
        }
        conn->closeCond->Wait(conn->lock);
    }
    conn->lock->Release();

    FreeConnection(connId);
    return E_SUCCESS;
}

bool ConnectionManager::IsValidConnection(const int connId) {
    Connection* conn = GetConnection(connId);
    return conn != nullptr && !conn->IsTerminated();
}

void ConnectionManager::PrintConnections() {
    printf("=== Connection Table ===\n");
    connectionsLock->Acquire();
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i] != nullptr && !connections[i]->IsTerminated()) {
            const auto&[remoteAddr, localPort, remotePort] = connections[i]->GetKey();
            printf("  [%d] %d:%d <-> %d:%d  State: %s\n",
                   i, GetLocalAddress(), localPort,
                   remoteAddr, remotePort,
                   ConnectionStateToString(connections[i]->GetState()));
        }
    }
    connectionsLock->Release();

    printf("=== Listener Table ===\n");
    listenersLock->Acquire();
    for (int i = 0; i < MAX_LISTENERS; i++) {
        if (listeners[i] != nullptr && listeners[i]->active) {
            printf("  [%d] Port %d, pending: %d\n",
                   i, listeners[i]->port, listeners[i]->pendingCount);
        }
    }
    listenersLock->Release();
}

int ConnectionManager::AllocateConnection() {
    connectionsLock->Acquire();

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i] == nullptr) {
            connections[i] = new Connection();
            connections[i]->Initialize(i, this);
            connectionsLock->Release();
            DEBUG('n', "Allocated connection slot %d\n", i);
            return i;
        }

        if (connections[i]->IsTerminated()) {
            connections[i]->Reset();
            connections[i]->Initialize(i, this);
            connectionsLock->Release();
            DEBUG('n', "Reusing connection slot %d\n", i);
            return i;
        }
    }

    connectionsLock->Release();
    return -1;
}

void ConnectionManager::FreeConnection(const int connId) {
    if (connId < 0 || connId >= MAX_CONNECTIONS) { return; }

    connectionsLock->Acquire();
    if (connections[connId] != nullptr) { connections[connId]->Reset(); }
    connectionsLock->Release();
}

Connection* ConnectionManager::GetConnection(const int connId) {
    if (connId < 0 || connId >= MAX_CONNECTIONS) { return nullptr; }

    connectionsLock->Acquire();
    Connection* conn = connections[connId];
    connectionsLock->Release();
    return conn;
}

Connection* ConnectionManager::FindConnectionByKey(const ConnectionKey& key) {
    connectionsLock->Acquire();

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i] != nullptr &&
            !connections[i]->IsTerminated() &&
            connections[i]->GetKey() == key) {
            connectionsLock->Release();
            return connections[i];
        }
    }

    connectionsLock->Release();
    return nullptr;
}

Connection* ConnectionManager::FindConnectionByLocalPort(const uint16_t localPort) {
    connectionsLock->Acquire();

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i] != nullptr &&
            !connections[i]->IsTerminated() &&
            connections[i]->GetKey().localPort == localPort) {
            connectionsLock->Release();
            return connections[i];
        }
    }

    connectionsLock->Release();
    return nullptr;
}

int ConnectionManager::AllocateListener(uint16_t port) {
    listenersLock->Acquire();

    for (int i = 0; i < MAX_LISTENERS; i++) {
        if (listeners[i] != nullptr && listeners[i]->active &&
            listeners[i]->port == port) {
            listenersLock->Release();
            return E_ADDRINUSE;
        }
    }

    for (int i = 0; i < MAX_LISTENERS; i++) {
        if (listeners[i] == nullptr) {
            listeners[i] = new Listener();
            listeners[i]->active = true;
            listeners[i]->port = port;
            listenersLock->Release();
            DEBUG('n', "Allocated listener slot %d for port %d\n", i, port);
            return i;
        }
        if (!listeners[i]->active) {
            listeners[i]->Reset();
            listeners[i]->active = true;
            listeners[i]->port = port;
            listenersLock->Release();
            DEBUG('n', "Reusing listener slot %d for port %d\n", i, port);
            return i;
        }
    }

    listenersLock->Release();
    return E_NOTCONN;
}

void ConnectionManager::FreeListener(const int listenerId) {
    if (listenerId < 0 || listenerId >= MAX_LISTENERS) { return; }

    listenersLock->Acquire();
    if (listeners[listenerId] != nullptr) { listeners[listenerId]->active = false; }
    listenersLock->Release();
}

Listener* ConnectionManager::GetListener(const int listenerId) {
    if (listenerId < 0 || listenerId >= MAX_LISTENERS) { return nullptr; }

    listenersLock->Acquire();
    Listener* listener = listeners[listenerId];
    listenersLock->Release();
    return listener;
}

Listener* ConnectionManager::FindListenerByPort(const uint16_t port) {
    listenersLock->Acquire();

    for (int i = 0; i < MAX_LISTENERS; i++) {
        if (listeners[i] != nullptr && listeners[i]->active &&
            listeners[i]->port == port) {
            listenersLock->Release();
            return listeners[i];
        }
    }

    listenersLock->Release();
    return nullptr;
}

uint16_t ConnectionManager::AllocateEphemeralPort() {
    portLock->Acquire();

    const uint16_t startPort = nextEphemeralPort;

    do {
        if (!IsPortInUse(nextEphemeralPort)) {
            const uint16_t port = nextEphemeralPort;
            nextEphemeralPort++;
            if (nextEphemeralPort > PORT_RANGE_EPHEMERAL_MAX) {
                nextEphemeralPort = PORT_RANGE_EPHEMERAL_MIN;
            }
            portLock->Release();
            return port;
        }

        nextEphemeralPort++;
        if (nextEphemeralPort > PORT_RANGE_EPHEMERAL_MAX) {
            nextEphemeralPort = PORT_RANGE_EPHEMERAL_MIN;
        }
    } while (nextEphemeralPort != startPort);

    portLock->Release();
    return 0;
}

bool ConnectionManager::IsPortInUse(const uint16_t port) const {
    for (int i = 0; i < MAX_LISTENERS; i++) {
        if (listeners[i] != nullptr && listeners[i]->active &&
            listeners[i]->port == port) {
            return true;
        }
    }

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i] != nullptr && !connections[i]->IsTerminated() &&
            connections[i]->GetKey().localPort == port) {
            return true;
        }
    }

    return false;
}

void ConnectionManager::ProcessIncomingPacket(PacketHeader pktHdr, const MailHeader mailHdr, const char* data) {
    if (mailHdr.length < sizeof(ReliableHeader)) {
        DEBUG('n', "Packet too small for reliable header\n");
        return;
    }

    const auto relHdr = (ReliableHeader*)data;
    const char* payload = data + sizeof(ReliableHeader);

    DEBUG('n', "ProcessIncomingPacket: %s from %d:%d to port %d seq=%u ack=%u (flags=0x%02x)\n",
          MessageTypeToString(relHdr->getType()), pktHdr.from, relHdr->srcPort, relHdr->dstPort, relHdr->seqNum, relHdr->ackNum, relHdr->getFlags());

    if (relHdr->getType() == MessageType::MSG_SYN) { HandleIncomingSYN(pktHdr, relHdr); }
    else { RouteToConnection(pktHdr, relHdr, payload); }
}

void ConnectionManager::HandleIncomingSYN(PacketHeader pktHdr, const ReliableHeader* relHdr) {
    DEBUG('n', "HandleIncomingSYN: from %d:%d to port %d\n", pktHdr.from, relHdr->srcPort, relHdr->dstPort);

    Listener* listener = FindListenerByPort(relHdr->dstPort);

    if (listener == nullptr) {
        DEBUG('n', "No listener on port %d, sending RST\n", relHdr->dstPort);
        SendRST(pktHdr.from, relHdr->dstPort, relHdr->srcPort, 0, relHdr->seqNum + 1);
        return;
    }

    ConnectionKey key;
    key.remoteAddr = pktHdr.from;
    key.localPort = relHdr->dstPort;
    key.remotePort = relHdr->srcPort;

    if (Connection* existingConn = FindConnectionByKey(key); existingConn != nullptr) {
        existingConn->HandleSYN(relHdr, pktHdr.from);
        return;
    }

    listener->lock->Acquire();
    if (listener->pendingCount >= MAX_PENDING_ACCEPTS) {
        listener->lock->Release();
        DEBUG('n', "Listener backlog full, dropping SYN\n");
        return;
    }

    const int connId = AllocateConnection();
    if (connId < 0) {
        listener->lock->Release();
        DEBUG('n', "No connection slots, dropping SYN\n");
        return;
    }

    Connection* conn = GetConnection(connId);
    conn->SetKey(pktHdr.from, relHdr->dstPort, relHdr->srcPort);
    conn->SetState(CONN_LISTEN);

    conn->HandleSYN(relHdr, pktHdr.from);

    for (int i = 0; i < MAX_PENDING_ACCEPTS; i++) {
        if (listener->pendingConnections[i] == nullptr) {
            listener->pendingConnections[i] = conn;
            listener->pendingCount++;
            break;
        }
    }

    listener->acceptCond->Signal(listener->lock);
    listener->lock->Release();
}

void ConnectionManager::RouteToConnection(PacketHeader pktHdr, const ReliableHeader* relHdr, const char* payload) {
    ConnectionKey key;
    key.remoteAddr = pktHdr.from;
    key.localPort = relHdr->dstPort;
    key.remotePort = relHdr->srcPort;

    Connection* conn = FindConnectionByKey(key);

    if (conn == nullptr) {
        if (relHdr->getType() != MessageType::MSG_RST) {
            DEBUG('n', "No connection for packet from %d:%d, sending RST\n", pktHdr.from, relHdr->srcPort);
            SendRST(pktHdr.from, relHdr->dstPort, relHdr->srcPort, relHdr->ackNum, relHdr->seqNum + 1);
        }
        return;
    }

    switch (relHdr->getType()) {
        case MessageType::MSG_SYN:
            conn->HandleSYN(relHdr, pktHdr.from);
            break;
        case MessageType::MSG_SYN_ACK:
            conn->HandleSYNACK(relHdr);
            break;
        case MessageType::MSG_ACK:
            conn->HandleACK(relHdr);
            break;
        case MessageType::MSG_DATA:
            conn->HandleDATA(relHdr, payload, relHdr->getFlags());
            break;
        case MessageType::MSG_FIN:
            conn->HandleFIN(relHdr);
            break;
        case MessageType::MSG_FIN_ACK:
            conn->HandleFINACK(relHdr);
            break;
        case MessageType::MSG_RST:
            conn->HandleRST(relHdr);
            break;
        case MessageType::MSG_CHUNK_BEGIN:
            conn->HandleDATA(relHdr, payload, relHdr->getFlags());
        default:
            DEBUG('n', "Unknown message type: %d\n", relHdr->getType());
            break;
    }
}

void ConnectionManager::SendPacket(NetworkAddress destAddr, const uint16_t srcPort,
                                   uint16_t dstPort, const MessageType type,
                                   uint32_t seqNum, uint32_t ackNum,
                                   const char* data, int dataLen, const uint8_t flags) {
    PacketHeader pktHdr;
    MailHeader mailHdr;
    char buffer[MaxMailSize];

    auto relHdr = reinterpret_cast<ReliableHeader*>(buffer);
    relHdr->srcPort = srcPort;
    relHdr->dstPort = dstPort;
    relHdr->seqNum = seqNum;
    relHdr->ackNum = ackNum;
    relHdr->dataLen = dataLen;
    relHdr->setType(type);
    relHdr->setFlags(flags);

    if (data != nullptr && dataLen > 0) {
        bcopy(data, buffer + sizeof(ReliableHeader), dataLen);
    }

    pktHdr.to = destAddr;
    pktHdr.from = GetLocalAddress();

    mailHdr.to = RELIABLE_MAILBOX;
    mailHdr.from = RELIABLE_MAILBOX;
    mailHdr.length = sizeof(ReliableHeader) + dataLen;

    DEBUG('n', "SendPacket: %s to %d:%d seq=%u ack=%u len=%d\n",
          MessageTypeToString(type), destAddr, dstPort, seqNum, ackNum, dataLen);

    postOffice->Send(pktHdr, mailHdr, buffer);
}

void ConnectionManager::SendRST(NetworkAddress destAddr, const uint16_t srcPort,
                                const uint16_t dstPort, const uint32_t seqNum,
                                const uint32_t ackNum) {
    SendPacket(destAddr, srcPort, dstPort, MessageType::MSG_RST, seqNum, ackNum, nullptr, 0, static_cast<uint8_t>(MessageFlag::FLAG_CONTROL));
}

void ConnectionManager::CheckAllTimeouts() {
    const long long currentTime = stats->totalTicks;

    connectionsLock->Acquire();
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i] != nullptr && !connections[i]->IsTerminated()) {
            connections[i]->CheckTimeouts(currentTime);
        }
    }
    connectionsLock->Release();
}

void ConnectionManager::WorkerThreadEntry(const int arg) {
    const auto mgr = (ConnectionManager*)arg;
    mgr->WorkerLoop();
}

void ConnectionManager::WorkerLoop() {
    DEBUG('n', "Connection worker thread started\n");

    PacketHeader pktHdr;
    MailHeader mailHdr;
    char buffer[MaxMailSize];

    while (!shutdownRequested) {
        if (postOffice->HasMessages(RELIABLE_MAILBOX)) {
            postOffice->Receive(RELIABLE_MAILBOX, &pktHdr, &mailHdr, buffer);
            ProcessIncomingPacket(pktHdr, mailHdr, buffer);
        }
        CheckAllTimeouts();
        currentThread->Yield();
    }

    DEBUG('n', "Connection worker thread exiting\n");
}