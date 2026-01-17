#ifndef NETPROTOCOL_H
#define NETPROTOCOL_H

#include "network.h"
#include "post.h"

#include <cstdint>

#define MAX_CONNECTIONS 16
#define MAX_LISTENERS 8
#define MAX_PENDING_MSGS 10
#define MAX_PENDING_ACCEPTS 4
#define MAX_RETRANSMISSIONS 10
#define MAX_CONNECT_ATTEMPTS 10

#define RELIABLE_TIMEOUT 10000
#define CONNECT_TIMEOUT 50000
#define DEFAULT_ACCEPT_TIMEOUT -1

// Maybe usefull one day
#define PORT_RANGE_RESERVED_MIN 1
#define PORT_RANGE_RESERVED_MAX 1023

#define PORT_RANGE_EPHEMERAL_MIN 1024
#define PORT_RANGE_EPHEMERAL_MAX 65535

#define PORT_AUTO 0

#define RELIABLE_MAILBOX 3 // 3 for no conflict with legacy tests

enum class MessageType : uint8_t {
    MSG_SYN     = 1,
    MSG_SYN_ACK = 2,
    MSG_ACK     = 3,
    MSG_DATA    = 4,
    MSG_FIN     = 5,
    MSG_FIN_ACK = 6,
    MSG_RST     = 7
};

inline const char* MessageTypeToString(const MessageType type) {
    switch (type) {
    case MessageType::MSG_SYN:       return "SYN";
    case MessageType::MSG_SYN_ACK:   return "SYN_ACK";
    case MessageType::MSG_ACK:       return "ACK";
    case MessageType::MSG_DATA:      return "DATA";
    case MessageType::MSG_FIN:       return "FIN";
    case MessageType::MSG_FIN_ACK:   return "FIN_ACK";
    case MessageType::MSG_RST:       return "RST";
    default:                         return "UNKNOWN";
    }
}

enum ConnectionState {
    // Initial states
    CONN_CLOSED,            // No connection exists
    CONN_LISTEN,            // Waiting for incoming connection (server)

    // Connection establishment
    CONN_SYN_SENT,          // SYN sent, waiting for SYN-ACK (client)
    CONN_SYN_RECEIVED,      // SYN received, SYN-ACK sent, waiting for ACK (server)

    // Established
    CONN_ESTABLISHED,       // Connection established, data transfer possible

    // Connection termination
    CONN_FIN_WAIT_1,        // FIN sent, waiting for ACK or FIN
    CONN_FIN_WAIT_2,        // FIN ACKed, waiting for peer's FIN
    CONN_CLOSE_WAIT,        // Received FIN, waiting for local close
    CONN_CLOSING,           // Both sides sent FIN simultaneously
    CONN_LAST_ACK,          // Waiting for final ACK after sending FIN
    CONN_TIME_WAIT,         // Waiting before final cleanup (2MSL)

    // Terminal state
    CONN_TERMINATED         // Connection fully closed, slot can be reused
};

inline const char* ConnectionStateToString(const ConnectionState state) {
    switch (state) {
    case CONN_CLOSED:       return "CLOSED";
    case CONN_LISTEN:       return "LISTEN";
    case CONN_SYN_SENT:     return "SYN_SENT";
    case CONN_SYN_RECEIVED: return "SYN_RECEIVED";
    case CONN_ESTABLISHED:  return "ESTABLISHED";
    case CONN_FIN_WAIT_1:   return "FIN_WAIT_1";
    case CONN_FIN_WAIT_2:   return "FIN_WAIT_2";
    case CONN_CLOSE_WAIT:   return "CLOSE_WAIT";
    case CONN_CLOSING:      return "CLOSING";
    case CONN_LAST_ACK:     return "LAST_ACK";
    case CONN_TIME_WAIT:    return "TIME_WAIT";
    case CONN_TERMINATED:   return "TERMINATED";
    default:                return "UNKNOWN";
    }
}

struct ReliableHeader {
    uint32_t seqNum;
    uint32_t ackNum;
    uint16_t srcPort;
    uint16_t dstPort;
    uint16_t dataLen;
    MessageType  type;

    ReliableHeader() : seqNum(0), ackNum(0), srcPort(0), dstPort(0), dataLen(0), type() {}
    ReliableHeader(const MessageType t, const uint16_t sPort, const uint16_t dPort, const uint32_t sNum, const uint32_t aNum, const uint16_t len)
        : seqNum(sNum), ackNum(aNum), srcPort(sPort), dstPort(dPort), dataLen(len), type(t) {}
};

#define MAX_RELIABLE_DATA (MaxMailSize - sizeof(ReliableHeader))

struct ConnectionKey {
    NetworkAddress remoteAddr;
    uint16_t localPort;
    uint16_t remotePort;

    bool operator==(const ConnectionKey& other) const {
        return remoteAddr == other.remoteAddr && localPort == other.localPort && remotePort == other.remotePort;
    }
};

struct PendingMessage {
    long long sentTime;
    uint32_t seqNum;
    int attempts;
    uint16_t dataLen;
    MessageType type;
    uint8_t flags;
    char data[MAX_RELIABLE_DATA];
};

constexpr uint8_t FLAG_ACTIVE = 1 << 0;
constexpr uint8_t FLAG_ACKED  = 1 << 1;

#endif