#include "usernetwork.h"

#include "connectionmanager.h"
#include "exception.h"
#include "nos_errno.h"
#include "network.h"
#include "system.h"
#include "nos_limits.h"
#include "syscall.h"
#include "thread.h"
#include "process.h"

#include <cstdint>

void handle_SC_connect() {
    int remoteAddr = machine->ReadRegister(4);
    int remotePort = machine->ReadRegister(5);
    int localPort = machine->ReadRegister(6);

    DEBUG('n', "SC_connect: remoteAddr=%d remotePort=%d localPort=%d\n", remoteAddr, remotePort, localPort);

    if (remoteAddr < 0 || remoteAddr > 9) { RETURN(-E_INVAL); }
    if (remotePort <= 0 || remotePort > 65535) { RETURN(-E_INVAL); }
    if (localPort < 0 || localPort > 65535) { RETURN(-E_INVAL); }

    ConnectionManager* mgr = GetConnectionManager();
    if (mgr == nullptr) { RETURN(-E_NOSYS); }

    const int result = mgr->Connect(static_cast<NetworkAddress>(remoteAddr),
                              static_cast<uint16_t>(remotePort),
                              static_cast<uint16_t>(localPort));

    if (result < 0) { RETURN(-result); }

    DEBUG('n', "SC_connect: success, connId=%d\n", result);
    RETURN(result);
}

void handle_SC_listen() {
    const int port = machine->ReadRegister(4);

    DEBUG('n', "SC_listen: port=%d\n", port);

    if (port <= 0 || port > 65535) { RETURN(-E_INVAL); }

    ConnectionManager* mgr = GetConnectionManager();
    if (mgr == nullptr) { RETURN(-E_NOSYS); }

    const int result = mgr->Listen(static_cast<uint16_t>(port));
    if (result < 0) { RETURN(-result); }

    DEBUG('n', "SC_listen: success, listenerId=%d\n", result);
    RETURN(result);
}

void handle_SC_accept() {
    int listenerId = machine->ReadRegister(4);
    int timeoutMs = machine->ReadRegister(5);

    DEBUG('n', "SC_accept: listenerId=%d timeoutMs=%d\n", listenerId, timeoutMs);

    if (listenerId < 0) { RETURN(-E_INVAL); }

    ConnectionManager* mgr = GetConnectionManager();
    if (mgr == nullptr) { RETURN(-E_NOSYS); }

    const int result = mgr->Accept(listenerId, timeoutMs);
    if (result < 0) { RETURN(-result); }

    DEBUG('n', "SC_accept: success, connId=%d\n", result);
    RETURN(result);
}

void handle_SC_sendto() {
    int connId = machine->ReadRegister(4);
    const int dataAddr = machine->ReadRegister(5);
    int size = machine->ReadRegister(6);

    GET_PROCESS_ADDRSPACE();

    VALIDATE_ARG(space->IsValidUserRange(dataAddr, size), E_FAULT);

    DEBUG('n', "SC_sendto: connId=%d dataAddr=0x%x size=%d\n", connId, dataAddr, size);
    if (connId < 0) { RETURN(-E_INVAL); }
    if (size <= 0) { RETURN(-E_INVAL); }


    ConnectionManager* mgr = GetConnectionManager();
    if (mgr == nullptr) { RETURN(-E_NOSYS); }

    int maxSize = static_cast<int>(MAX_PUT_STRING);
    
    if (size > maxSize) {
        int result = mgr->Send(connId, NULL, -1);

        if (result < 0) { RETURN(-result); }

        int currentSize = size;
        int offset = 0;

        while (currentSize > 0) {
            int sizeToSend = (currentSize > maxSize) ? maxSize : currentSize;

            auto data = new char[sizeToSend];
            if (!CopyFromUserRaw(data, dataAddr + offset, sizeToSend)) {
                delete[] data;
                RETURN(-E_FAULT);
            }

            result = mgr->Send(connId, data, sizeToSend);
            delete[] data;

            if (result < 0) { RETURN(-result); }

            offset += sizeToSend;
            currentSize -= sizeToSend;
        }

        result = mgr->Send(connId, nullptr, -2);
        if (result < 0) { RETURN(-result); }

        RETURN(size);
    }

    auto data = new char[size];
    if (!CopyFromUserRaw(data, dataAddr, size)) {
        delete[] data;
        RETURN(-E_FAULT);
    }

    const int result = mgr->Send(connId, data, size);
    delete[] data;

    if (result < 0) { RETURN(-result); }

    DEBUG('n', "SC_sendto: success, sent=%d bytes\n", result);
    RETURN(result);
}

void handle_SC_recvfrom() {
    int connId = machine->ReadRegister(4);
    const int bufferAddr = machine->ReadRegister(5);
    int size = machine->ReadRegister(6);

    GET_PROCESS_ADDRSPACE();
    VALIDATE_ARG(space->IsValidUserRange(bufferAddr, size), E_FAULT);

    DEBUG('n', "SC_recvfrom: connId=%d bufferAddr=0x%x size=%d\n", connId, bufferAddr, size);

    if (connId < 0) { RETURN(-E_INVAL); }
    if (size <= 0) { RETURN(-E_INVAL); }

    ConnectionManager* mgr = GetConnectionManager();
    if (mgr == nullptr) { RETURN(-E_NOSYS); }

    int bufSize = static_cast<int>(MAX_PUT_STRING);
    if (bufSize > size) bufSize = size;
    char* buffer = new char[bufSize];

    MessageType msgType = MessageType::MSG_DATA;
    int result = mgr->Recv(connId, buffer, bufSize, &msgType);

    DEBUG('n', "SC_recvfrom: first Recv returned %d bytes, msgType=%d\n", result, static_cast<int>(msgType));

    if (result < 0) { delete[] buffer; RETURN(-result); }

    int totalReceived = 0;

    if (msgType == MessageType::MSG_CHUNK_BEGIN) {
        int userOffset = 0;
        bool chunkComplete = false;

        while (!chunkComplete) {
            msgType = MessageType::MSG_DATA;
            result = mgr->Recv(connId, buffer, bufSize, &msgType);

            if (result < 0) { delete[] buffer; RETURN(-result); }

            if (msgType == MessageType::MSG_CHUNK_END) {
                chunkComplete = true;
                break;
            }

            if (result > 0) {
                int spaceLeft = size - userOffset;
                int toCopy = (result < spaceLeft) ? result : spaceLeft;

                if (toCopy > 0) {
                    if (!CopyToUserRaw(bufferAddr + userOffset, buffer, toCopy)) {
                        delete[] buffer;
                        RETURN(-E_FAULT);
                    }
                    userOffset += toCopy;
                }

                if (spaceLeft <= result) {
                    DEBUG('n', "SC_recvfrom: user buffer full, stopping at %d bytes\n", userOffset);
                    while (msgType != MessageType::MSG_CHUNK_END) {
                        msgType = MessageType::MSG_DATA;
                        result = mgr->Recv(connId, buffer, bufSize, &msgType);
                        if (result < 0 || msgType == MessageType::MSG_CHUNK_END) break;
                    }
                    chunkComplete = true;
                }
            }
        }

        totalReceived = userOffset;
    } else {
        if (result > 0) {
            int toCopy = (result < size) ? result : size;
            if (!CopyToUserRaw(bufferAddr, buffer, toCopy)) {
                delete[] buffer;
                RETURN(-E_FAULT);
            }
            totalReceived = toCopy;
        } else {
            totalReceived = result;
        }
    }

    delete[] buffer;
    DEBUG('n', "SC_recvfrom: success, received=%d bytes\n", totalReceived);
    RETURN(totalReceived);
}

void handle_SC_close() {
    int id = machine->ReadRegister(4);

    DEBUG('n', "SC_close: id=%d\n", id);

    if (id < 0) { RETURN(-E_INVAL); }

    ConnectionManager* mgr = GetConnectionManager();
    if (mgr == nullptr) { RETURN(-E_NOSYS); }

    int result = mgr->Close(id);
    if (result == E_INVAL) { result = mgr->CloseListener(id); }
    if (result < 0) { RETURN(-result); }

    DEBUG('n', "SC_close: success\n");
    RETURN(E_SUCCESS);
}