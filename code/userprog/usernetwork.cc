#include "usernetwork.h"

#include "exception.h"
#include "post.h"
#include "nos_errno.h"
#include "system.h"
#include "nos_limits.h"
#include "syscall.h"

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

void handle_SC_connect() {
    // TODO: implement
    RETURN(E_SUCCESS);
}

void handle_SC_accept() {
    // TODO: implement
    RETURN(E_SUCCESS);
}

void handle_SC_sendto() {
    int to = machine->ReadRegister(4);
    int data_addr = machine->ReadRegister(5);
    int size = machine->ReadRegister(6);

    PacketHeader pktHdr;
    MailHeader mailHdr;
    char* data = new char[size];

    if (size > static_cast<int>(MaxMailSize) || size < 0) {
        delete[] data;
        RETURN(E_INVAL);
    }

    if (!CopyFromUserRaw(data, data_addr, size)) {
        delete[] data;
        RETURN(E_FAULT);
    }

    pktHdr.to = to;
    pktHdr.from = postOffice->GetNetAddr();
    pktHdr.length = size + sizeof(MailHeader);

    mailHdr.to = MAIL_BOX;
    mailHdr.from = MAIL_BOX;
    mailHdr.length = size;

    postOffice->Send(pktHdr, mailHdr, data);
    delete[] data;
    RETURN(E_SUCCESS);
}

void handle_SC_recvfrom() {
    DEBUG('n', "handle_SC_recvfrom called\n");

    int from = machine->ReadRegister(4);
    int data_addr = machine->ReadRegister(5);
    int size = machine->ReadRegister(6);

    PacketHeader pktHdr;
    MailHeader mailHdr;
    char* data = new char[MaxMailSize];

    postOffice->Receive(MAIL_BOX, &pktHdr, &mailHdr, data);

    if (pktHdr.from != from) {
        delete[] data;
        RETURN(-E_INVAL);
    }

    DEBUG('n', "Received data: %.*s\n", mailHdr.length, data);
    DEBUG('n', "param of recvfrom: from=%d, size=%d\n", from, size);
    if (!CopyToUserRaw(data_addr, data, MIN(size, static_cast<int>(mailHdr.length)))) {
        delete[] data;
        RETURN(-E_FAULT);
    }

    delete[] data;
    RETURN(static_cast<int>(mailHdr.length));
}