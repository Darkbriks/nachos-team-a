#include "connection.h"
#include "connectionmanager.h"
#include "system.h"
#include "thread.h"
#include "nos_errno.h"

#include <strings.h>

ReceivedData::ReceivedData(const char* src, const int len) {
    length = len;
    offset = 0;
    data = new char[len];
    bcopy(src, data, len);
}

ReceivedData::~ReceivedData() {
    delete[] data;
}

Connection::Connection() : recvQueue(new SynchList()),
                           lock(new Lock("connection lock")),
                           connectCond(new Condition("connect condition")),
                           acceptCond(new Condition("accept condition")),
                           recvCond(new Condition("receive condition")),
                           sendCond(new Condition("send condition")),
                           closeCond(new Condition("close condition")) {
    bzero(pending, sizeof(pending));
}

Connection::~Connection() {
    delete recvQueue;
    delete lock;
    delete connectCond;
    delete acceptCond;
    delete recvCond;
    delete sendCond;
    delete closeCond;
}

ConnectionState Connection::GetState() {
    lock->Acquire();
    const ConnectionState s = state;
    lock->Release();
    return s;
}

bool Connection::CanSend() {
    lock->Acquire();
    const bool result = (state == CONN_ESTABLISHED || state == CONN_CLOSE_WAIT);
    lock->Release();
    return result;
}

bool Connection::CanReceive() {
    lock->Acquire();
    const bool result = (state == CONN_ESTABLISHED || state == CONN_FIN_WAIT_1 || state == CONN_FIN_WAIT_2);
    lock->Release();
    return result;
}

int Connection::QueueSend(const char* data, int length) {
    lock->Acquire();

    if (state != CONN_ESTABLISHED && state != CONN_CLOSE_WAIT) {
        lock->Release();
        return E_NOTCONN;
    }

    if (length <= 0 || length > static_cast<int>(MAX_RELIABLE_DATA)) {
        lock->Release();
        return E_INVAL;
    }

    PendingMessage* slot = nullptr;
    while ((slot = GetFreePendingSlot()) == nullptr) {
        DEBUG('n', "[Conn %d] Send buffer full, waiting...\n", connId);
        sendCond->Wait(lock);

        if (state != CONN_ESTABLISHED && state != CONN_CLOSE_WAIT) {
            lock->Release();
            return E_CLOSED;
        }
    }

    const uint32_t seqNum = sendSeqNum++;
    slot->sentTime = 0;
    slot->seqNum = seqNum;
    slot->attempts = 0;
    slot->dataLen = length;
    slot->type = MessageType::MSG_DATA;
    slot->flags |= FLAG_ACTIVE;
    slot->flags &= ~FLAG_ACKED;

    bcopy(data, slot->data, length);
    pendingCount++;

    TransmitPending(slot);

    lock->Release();
    return seqNum;
}

int Connection::Read(char* buffer, int maxLength) {
    lock->Acquire();

    while (recvQueue->IsEmpty()) {
        if (state == CONN_CLOSE_WAIT || state == CONN_CLOSING ||
            state == CONN_TIME_WAIT || state == CONN_TERMINATED ||
            state == CONN_CLOSED) {
            lock->Release();
            return 0;
        }

        recvCond->Wait(lock);
    }

    lock->Release();

    ReceivedData* rd = static_cast<ReceivedData*>(recvQueue->Remove());

    // TODO: For now, it's UDP like
    
    int available = rd->length - rd->offset;
    const int toCopy = (available < maxLength) ? available : maxLength;

    // TODO : au cas ou pour retrouver le coupable
    int i = 0;
    while (rd->data[i] != EOF || i < toCopy) {
        buffer[bufferPosition + i] = rd->data[i]+rd->offset;
        i++;
    }

    // bcopy(rd->data + rd->offset, buffer, toCopy);
    bufferPosition += toCopy;

    delete rd;
    return toCopy;
}

bool Connection::HasDataAvailable() {
    return !recvQueue->IsEmpty();
}

void Connection::InitiateClose() {
    lock->Acquire();

    if (state == CONN_ESTABLISHED) {
        SetState(CONN_FIN_WAIT_1);
        SendControlMessage(MessageType::MSG_FIN, sendSeqNum++, recvSeqNum);
    } else if (state == CONN_CLOSE_WAIT) {
        SetState(CONN_LAST_ACK);
        SendControlMessage(MessageType::MSG_FIN, sendSeqNum++, recvSeqNum);
    }

    lock->Release();
}

bool Connection::IsTerminated() {
    lock->Acquire();
    const bool result = (state == CONN_TERMINATED || state == CONN_CLOSED);
    lock->Release();
    return result;
}

void Connection::Reset() {
    lock->Acquire();

    state = CONN_CLOSED;
    key.remoteAddr = 0;
    key.localPort = 0;
    key.remotePort = 0;

    sendSeqNum = 0;
    sendAckNum = 0;
    recvSeqNum = 0;
    initialSeqNum = 0;
    pendingCount = 0;

    for (int i = 0; i < MAX_PENDING_MSGS; i++) {
        pending[i].flags = 0;
    }

    while (!recvQueue->IsEmpty()) {
        const ReceivedData* rd = static_cast<ReceivedData*>(recvQueue->Remove());
        delete rd;
    }

    lock->Release();
}

void Connection::Initialize(const int id, ConnectionManager* mgr) {
    connId = id;
    manager = mgr;
    state = CONN_CLOSED;

    lastActivityTime = stats->totalTicks;
}

void Connection::SetKey(NetworkAddress remote, const uint16_t local, const uint16_t remotePort) {
    key.remoteAddr = remote;
    key.localPort = local;
    key.remotePort = remotePort;
}

void Connection::SetState(const ConnectionState newState) {
    DEBUG('n', "[Conn %d] State transition: %s -> %s\n", connId, ConnectionStateToString(state), ConnectionStateToString(newState));
    state = newState;
}

PendingMessage* Connection::GetFreePendingSlot() {
    for (int i = 0; i < MAX_PENDING_MSGS; i++) {
        if (!(pending[i].flags & FLAG_ACTIVE)) {
            bzero(&pending[i], sizeof(PendingMessage));
            return &pending[i];
        }
    }
    return nullptr;
}

PendingMessage* Connection::FindPending(const uint32_t seqNum) {
    for (int i = 0; i < MAX_PENDING_MSGS; i++) {
        if ((pending[i].flags & FLAG_ACTIVE) && pending[i].seqNum == seqNum) {
            return &pending[i];
        }
    }
    return nullptr;
}

void Connection::AcknowledgeMessage(uint32_t seqNum) {
    if (PendingMessage* msg = FindPending(seqNum); msg != nullptr) {
        if (!(msg->flags & FLAG_ACKED)) {
            DEBUG('n', "[Conn %d] ACK received for seq %u\n", connId, seqNum);
            msg->flags |= FLAG_ACKED;
            sendCond->Broadcast(lock);
        }
    }
}

void Connection::EnqueueReceivedData(const char* data, const int length) {
    auto rd = new ReceivedData(data, length);

    lock->Release();
    recvQueue->Append(rd);
    lock->Acquire();

    recvCond->Broadcast(lock);
}

uint32_t Connection::GenerateInitialSeqNum() {
    // TODO: Find better
    return static_cast<uint32_t>(stats->totalTicks & 0xFFFFFFFF) + connId * 1000;
}

void Connection::HandleSYN(const ReliableHeader* hdr, NetworkAddress from) {
    lock->Acquire();

    DEBUG('n', "[Conn %d] Received SYN from machine %d, seq=%u\n", connId, from, hdr->seqNum);

    if (state == CONN_LISTEN || state == CONN_SYN_SENT) {
        recvSeqNum = hdr->seqNum + 1;

        if (state == CONN_LISTEN) {
            initialSeqNum = GenerateInitialSeqNum();
            sendSeqNum = initialSeqNum;
        }

        SetState(CONN_SYN_RECEIVED);
        SendControlMessage(MessageType::MSG_SYN_ACK, sendSeqNum++, recvSeqNum);

        acceptCond->Signal(lock);
    } else if (state == CONN_SYN_RECEIVED || state == CONN_ESTABLISHED) {
        SendControlMessage(MessageType::MSG_SYN_ACK, initialSeqNum, recvSeqNum);
    }

    lock->Release();
}

void Connection::HandleSYNACK(const ReliableHeader* hdr) {
    lock->Acquire();

    DEBUG('n', "[Conn %d] Received SYN-ACK, seq=%u ack=%u\n", connId, hdr->seqNum, hdr->ackNum);

    if (state == CONN_SYN_SENT) {
        if (hdr->ackNum == initialSeqNum + 1) {
            recvSeqNum = hdr->seqNum + 1;
            sendSeqNum = hdr->ackNum;

            SetState(CONN_ESTABLISHED);
            SendControlMessage(MessageType::MSG_ACK, sendSeqNum, recvSeqNum);

            connectCond->Signal(lock);
        }
    } else if (state == CONN_SYN_RECEIVED) {
        recvSeqNum = hdr->seqNum + 1;
        SetState(CONN_ESTABLISHED);
        SendControlMessage(MessageType::MSG_ACK, sendSeqNum, recvSeqNum);
        connectCond->Signal(lock);
    }

    lock->Release();
}

void Connection::HandleACK(const ReliableHeader* hdr) {
    lock->Acquire();

    DEBUG('n', "[Conn %d] Received ACK, ack=%u\n", connId, hdr->ackNum);

    AcknowledgeMessage(hdr->ackNum - 1);

    switch (state) {
        case CONN_SYN_RECEIVED:
            SetState(CONN_ESTABLISHED);
            connectCond->Signal(lock);
            acceptCond->Signal(lock);
            break;

        case CONN_FIN_WAIT_1:
            SetState(CONN_FIN_WAIT_2);
            break;

        case CONN_CLOSING:
            SetState(CONN_TIME_WAIT);
            closeCond->Signal(lock);
            break;

        case CONN_LAST_ACK:
            SetState(CONN_TERMINATED);
            closeCond->Signal(lock);
            break;

        default:
            break;
    }

    lock->Release();
}

void Connection::HandleDATA(const ReliableHeader* hdr, const char* payload) {
    lock->Acquire();

    DEBUG('n', "[Conn %d] Received DATA seq=%u len=%u\n", connId, hdr->seqNum, hdr->dataLen);

    if (state != CONN_ESTABLISHED && state != CONN_FIN_WAIT_1 &&
        state != CONN_FIN_WAIT_2) {
        lock->Release();
        return;
    }

    if (hdr->seqNum == recvSeqNum) {
        recvSeqNum++;
        EnqueueReceivedData(payload, hdr->dataLen);
        SendACK();
    } else if (hdr->seqNum < recvSeqNum) {
        SendACK();
    }

    lock->Release();
}

void Connection::HandleFIN(const ReliableHeader* hdr) {
    lock->Acquire();

    DEBUG('n', "[Conn %d] Received FIN seq=%u\n", connId, hdr->seqNum);

    recvSeqNum = hdr->seqNum + 1;

    switch (state) {
        case CONN_ESTABLISHED:
            SetState(CONN_CLOSE_WAIT);
            SendControlMessage(MessageType::MSG_ACK, sendSeqNum, recvSeqNum);
            recvCond->Broadcast(lock);
            break;

        case CONN_FIN_WAIT_1:
            SetState(CONN_CLOSING);
            SendControlMessage(MessageType::MSG_ACK, sendSeqNum, recvSeqNum);
            break;

        case CONN_FIN_WAIT_2:
            SetState(CONN_TIME_WAIT);
            SendControlMessage(MessageType::MSG_ACK, sendSeqNum, recvSeqNum);
            closeCond->Signal(lock);
            break;

        default:
            SendControlMessage(MessageType::MSG_ACK, sendSeqNum, recvSeqNum);
            break;
    }

    lock->Release();
}

void Connection::HandleFINACK(const ReliableHeader* hdr) {
    lock->Acquire();

    DEBUG('n', "[Conn %d] Received FIN-ACK\n", connId);

    HandleACK(hdr);
    lock->Release();
    HandleFIN(hdr);
}

void Connection::HandleRST(const ReliableHeader* hdr) {
    lock->Acquire();

    DEBUG('n', "[Conn %d] Received RST\n", connId);

    SetState(CONN_TERMINATED);

    connectCond->Broadcast(lock);
    acceptCond->Broadcast(lock);
    recvCond->Broadcast(lock);
    sendCond->Broadcast(lock);
    closeCond->Broadcast(lock);

    lock->Release();
}

void Connection::SendControlMessage(const MessageType type, uint32_t seqNum, uint32_t ackNum) {
    DEBUG('n', "[Conn %d] Sending %s seq=%u ack=%u to machine %d port %d\n",
          connId, MessageTypeToString(type), seqNum, ackNum, key.remoteAddr, key.remotePort);

    manager->SendPacket(key.remoteAddr, key.localPort, key.remotePort,
                       type, seqNum, ackNum, nullptr, 0);

    lastActivityTime = stats->totalTicks;
}

void Connection::TransmitPending(PendingMessage* msg) {
    if (msg->attempts >= MAX_RETRANSMISSIONS) {
        DEBUG('n', "[Conn %d] Message seq %u failed after %d attempts\n", connId, msg->seqNum, msg->attempts);
        msg->flags &= ~FLAG_ACTIVE;
        pendingCount--;
        return;
    }

    if (msg->attempts > 0) {
        DEBUG('n', "[Conn %d] Retransmitting seq %u (attempt %d)\n", connId, msg->seqNum, msg->attempts + 1);
    }

    manager->SendPacket(key.remoteAddr, key.localPort, key.remotePort,
                       MessageType::MSG_DATA, msg->seqNum, recvSeqNum, msg->data, msg->dataLen);

    msg->attempts++;
    msg->sentTime = stats->totalTicks;
    lastActivityTime = stats->totalTicks;
}

void Connection::SendACK() {
    SendControlMessage(MessageType::MSG_ACK, sendSeqNum, recvSeqNum);
}

bool Connection::CheckTimeouts(const long long currentTime) {
    lock->Acquire();

    if (state == CONN_SYN_SENT || state == CONN_SYN_RECEIVED) {
        if (currentTime - connectStartTime > CONNECT_TIMEOUT) {
            connectAttempts++;
            if (connectAttempts >= MAX_CONNECT_ATTEMPTS) {
                DEBUG('n', "[Conn %d] Connection timeout after %d attempts\n", connId, connectAttempts);
                SetState(CONN_TERMINATED);
                connectCond->Broadcast(lock);
                lock->Release();
                return true;
            }

            if (state == CONN_SYN_SENT) {
                SendControlMessage(MessageType::MSG_SYN, initialSeqNum, 0);
            } else {
                SendControlMessage(MessageType::MSG_SYN_ACK, initialSeqNum, recvSeqNum);
            }
            connectStartTime = currentTime;
        }
    }

    for (int i = 0; i < MAX_PENDING_MSGS; i++) {
        if (PendingMessage* msg = &pending[i]; (msg->flags & FLAG_ACTIVE) && !(msg->flags & FLAG_ACKED)) {
            if (currentTime - msg->sentTime > RELIABLE_TIMEOUT) {
                TransmitPending(msg);
            }
        }
    }

    lock->Release();
    return false;
}