// sysdep.cc
//      Implementation of system-dependent interface.  Nachos uses the
//      routines defined here, rather than directly calling the UNIX library,
//      to simplify porting between versions of UNIX, and even to
//      other systems, such as MSDOS.
//
//      On UNIX, almost all of these routines are simple wrappers
//      for the underlying UNIX system calls.
//
//      NOTE: all of these routines refer to operations on the underlying
//      host machine (e.g., the DECstation, SPARC, etc.), supporting the
//      Nachos simulation code.  Nachos implements similar operations,
//      (such as opening a file), but those are implemented in terms
//      of hardware devices, which are simulated by calls to the underlying
//      routines in the host workstation OS.
//
//      This file includes lots of calls to C routines.  C++ requires
//      us to wrap all C definitions with a "extern "C" block".
//      This prevents the internal forms of the names from being
//      changed by the C++ compiler.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

extern "C" {
#include <errno.h> // modif norme ansi
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

// UNIX routines called by procedures in this file

#ifdef HOST_SNAKE
// int creat(char *name, unsigned short mode);
// int open(const char *name, int flags, ...);
#else
#if !defined(SOLARIS) && !defined(LINUX)
int creat(const char *name, unsigned short mode);
int open(const char *name, int flags, ...);
// void signal(int sig, VoidFunctionPtr func); -- this may work now!
#ifdef HOST_i386
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
           struct timeval *timeout);
#else
int select(int numBits, void *readFds, void *writeFds, void *exceptFds,
           struct timeval *timeout);
#endif
#endif
#endif

#if !defined(SOLARIS) && !defined(LINUX)
int unlink(char *name);
int read(int filedes, char *buf, int numBytes);
int write(int filedes, char *buf, int numBytes);
int lseek(int filedes, int offset, int whence);
int tell(int filedes);
int close(int filedes);
int unlink(char *name);

// definition varies slightly from platform to platform, so don't
// define unless gcc complains
// extern int recvfrom(int s, void *buf, int len, int flags, void *from, int
// *fromlen); 
//extern int sendto(int s, void *msg, int len, int flags, void *to,int tolen);

void srand(unsigned seed);
int rand(void);
unsigned sleep(unsigned);
void abort();
void exit();
int mprotect(char *addr, int len, int prot);

int socket(int, int, int);
int bind(int, const void *, int);
int recvfrom(int, void *, int, int, void *, int *);
// int sendto(int, const void *, int, int, void *, int);
#endif
}

#include "interrupt.h"
#include "system.h"

//----------------------------------------------------------------------
// TCP/IP Network Configuration
//----------------------------------------------------------------------

// Enable TCP/IP networking instead of Unix domain sockets
// Set to 0 to use Unix domain sockets (local machine only)
// Set to 1 to use TCP/IP sockets (cross-machine communication)
#define USE_TCPIP_NETWORK 1

#define BASE_PORT 9000        // Base port for NachOS network
#define MAX_MACHINES 10       // Maximum number of machines

// Machine address mapping: maps machine ID to IP address
// Default is localhost for all machines (for local testing)
// Modify these for cross-machine communication
static const char* machineHosts[MAX_MACHINES] = {
    "127.0.0.1",  // Machine 0
    "127.0.0.1",  // Machine 1
    "127.0.0.1",  // Machine 2
    "127.0.0.1",  // Machine 3
    "127.0.0.1",  // Machine 4
    "127.0.0.1",  // Machine 5
    "127.0.0.1",  // Machine 6
    "127.0.0.1",  // Machine 7
    "127.0.0.1",  // Machine 8
    "127.0.0.1"   // Machine 9
};

//----------------------------------------------------------------------
// PollFile
//      Check open file or open socket to see if there are any
//      characters that can be read immediately.  If so, read them
//      in, and return TRUE.
//
//      In the network case, if there are no threads for us to run,
//      and no characters to be read,
//      we need to give the other side a chance to get our host's CPU
//      (otherwise, we'll go really slowly, since UNIX time-slices
//      infrequently, and this would be like busy-waiting).  So we
//      delay for a short fixed time, before allowing ourselves to be
//      re-scheduled (sort of like a Yield, but cast in terms of UNIX).
//
//      "fd" -- the file descriptor of the file to be polled
//----------------------------------------------------------------------

bool PollFile(int fd) {
    int rfd = (1 << fd), wfd = 0, xfd = 0, retVal;
    struct timeval pollTime;

    // decide how long to wait if there are no characters on the file
    pollTime.tv_sec = 0;
    if (interrupt->getStatus() == IdleMode)
        pollTime.tv_usec = 20000; // delay to let other nachos run
    else
        pollTime.tv_usec = 0; // no delay

// poll file or socket
#if defined(HOST_i386) || defined(SOLARIS)
    retVal =
        select(32, (fd_set *)&rfd, (fd_set *)&wfd, (fd_set *)&xfd, &pollTime);
#else
    retVal = select(32, &rfd, &wfd, &xfd, &pollTime);
#endif

    ASSERT((retVal == 0) || (retVal == 1));
    if (retVal == 0)
        return FALSE; // no char waiting to be read
    return TRUE;
}

//----------------------------------------------------------------------
// OpenForWrite
//      Open a file for writing.  Create it if it doesn't exist; truncate it
//      if it does already exist.  Return the file descriptor.
//
//      "name" -- file name
//----------------------------------------------------------------------

int OpenForWrite(const char *name) {
    int fd = open(name, O_RDWR | O_CREAT | O_TRUNC, 0666);

    ASSERT(fd >= 0);
    return fd;
}

//----------------------------------------------------------------------
// OpenForReadWrite
//      Open a file for reading or writing.
//      Return the file descriptor, or error if it doesn't exist.
//
//      "name" -- file name
//----------------------------------------------------------------------

int OpenForReadWrite(const char *name, bool crashOnError) {
    int fd = open(name, O_RDWR, 0);

    ASSERT(!crashOnError || fd >= 0);
    return fd;
}

//----------------------------------------------------------------------
// Read
//      Read characters from an open file.  Abort if read fails.
//----------------------------------------------------------------------

void Read(int fd, char *buffer, int nBytes) {
    int retVal = read(fd, buffer, nBytes);
    ASSERT(retVal == nBytes);
}

//----------------------------------------------------------------------
// ReadPartial
//      Read characters from an open file, returning as many as are
//      available.
//----------------------------------------------------------------------

int ReadPartial(int fd, char *buffer, int nBytes) {
    return read(fd, buffer, nBytes);
}

//----------------------------------------------------------------------
// WriteFile
//      Write characters to an open file.  Abort if write fails.
//----------------------------------------------------------------------

void WriteFile(int fd, const char *buffer, int nBytes) {
    int retVal = write(fd, buffer, nBytes);
    ASSERT(retVal == nBytes);
}

//----------------------------------------------------------------------
// Lseek
//      Change the location within an open file.  Abort on error.
//----------------------------------------------------------------------

void Lseek(int fd, int offset, int whence) {
    int retVal = lseek(fd, offset, whence);
    ASSERT(retVal >= 0);
}

//----------------------------------------------------------------------
// Tell
//      Report the current location within an open file.
//----------------------------------------------------------------------

int Tell(int fd) {
#if defined(HOST_i386) || defined(SOLARIS)
    return lseek(fd, 0, SEEK_CUR); // 386BSD doesn't have the tell() system call
#else
    return tell(fd);
#endif
}

//----------------------------------------------------------------------
// Close
//      Close a file.  Abort on error.
//----------------------------------------------------------------------

void Close(int fd) {
    int retVal = close(fd);
    ASSERT(retVal >= 0);
}

//----------------------------------------------------------------------
// Unlink
//      Delete a file.
//----------------------------------------------------------------------

bool Unlink(const char *name) { return unlink(name); }

//----------------------------------------------------------------------
// OpenSocket
//      Open an interprocess communication (IPC) connection.  For now,
//      just open a datagram port where other Nachos (simulating
//      workstations on a network) can send messages to this Nachos.
//
//      Uses TCP/IP sockets if USE_TCPIP_NETWORK is enabled,
//      otherwise uses Unix domain sockets.
//----------------------------------------------------------------------

int OpenSocket() {
    int sockID;

#if USE_TCPIP_NETWORK
    // Create TCP/IP UDP socket for cross-machine communication
    sockID = socket(AF_INET, SOCK_DGRAM, 0);
    ASSERT(sockID >= 0);

    // Set socket options to allow address reuse
    int optval = 1;
    setsockopt(sockID, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
#else
    // Create Unix domain socket for local-only communication
    sockID = socket(AF_UNIX, SOCK_DGRAM, 0);
    ASSERT(sockID >= 0);
#endif

    return sockID;
}

//----------------------------------------------------------------------
// CloseSocket
//      Close the IPC connection.
//----------------------------------------------------------------------

void CloseSocket(int sockID) { (void)close(sockID); }

//----------------------------------------------------------------------
// InitSocketName
//      Initialize a UNIX socket address -- magical!
//      Only needed for Unix domain sockets.
//----------------------------------------------------------------------

#if !USE_TCPIP_NETWORK
static void InitSocketName(struct sockaddr_un *uname, const char *name) {
    uname->sun_family = AF_UNIX;
    strcpy(uname->sun_path, name);
}
#endif

//----------------------------------------------------------------------
// AssignNameToSocket
//      Give a UNIX file name to the IPC port, so other instances of Nachos
//      can locate the port.
//
//      For TCP/IP: binds to a specific port based on machine ID
//      For Unix: binds to a Unix domain socket file
//----------------------------------------------------------------------

void AssignNameToSocket(const char *socketName, int sockID) {
#if USE_TCPIP_NETWORK
    // Extract machine ID from socket name (format: "SOCKET_N")
    int machineID = atoi(socketName + 7);  // Skip "SOCKET_" prefix

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(BASE_PORT + machineID);
    addr.sin_addr.s_addr = INADDR_ANY;  // Listen on all interfaces

    int retVal = bind(sockID, (struct sockaddr *)&addr, sizeof(addr));
    if (retVal < 0) {
        perror("bind failed");
        printf("Failed to bind to port %d (machine %d)\n", BASE_PORT + machineID, machineID);
    }
    ASSERT(retVal >= 0);
    DEBUG('n', "Created TCP/IP socket on port %d\n", BASE_PORT + machineID);
#else
    // Unix domain socket (original behavior)
    struct sockaddr_un uName;
    int retVal;

    (void)unlink(socketName); // in case it's still around from last time

    InitSocketName(&uName, socketName);
    retVal = bind(sockID, (struct sockaddr *)&uName, sizeof(uName));
    ASSERT(retVal >= 0);
    DEBUG('n', "Created socket %s\n", socketName);
#endif
}

//----------------------------------------------------------------------
// DeAssignNameToSocket
//      Delete the UNIX file name we assigned to our IPC port, on cleanup.
//      For TCP/IP sockets, this is a no-op.
//----------------------------------------------------------------------
void DeAssignNameToSocket(const char *socketName) {
#if !USE_TCPIP_NETWORK
    (void)unlink(socketName);
#endif
}

//----------------------------------------------------------------------
// PollSocket
//      Return TRUE if there are any messages waiting to arrive on the
//      IPC port.
//----------------------------------------------------------------------
bool PollSocket(int sockID) {
    return PollFile(sockID); // on UNIX, socket ID's are just file ID's
}

//----------------------------------------------------------------------
// ReadFromSocket
//      Read a fixed size packet off the IPC port.  Abort on error.
//----------------------------------------------------------------------
void ReadFromSocket(int sockID, char *buffer, int packetSize) {
    int retVal;
    /* extern int errno; modif norme ANSI */

#if USE_TCPIP_NETWORK
    // TCP/IP socket
    struct sockaddr_in addr;
    socklen_t size = sizeof(addr);

    retVal = recvfrom(sockID, buffer, packetSize, 0, (struct sockaddr *)&addr, &size);
#else
    // Unix domain socket
    struct sockaddr_un uName;

    // LB: Signedness problem on Solaris 5.6/SPARC, as the last
    // parameter of recvfrom is specified as a int *. In the later
    // versions, it is specified as a void *. Casting size to int instead
    // of unsigned seems to fix the problem, but it is admittingly
    // rather ad-hoc...
#ifndef SOLARIS
    unsigned int size = sizeof(uName);
#else
    int size = (int)sizeof(uName);
#endif
    // End of correction.

    retVal = recvfrom(sockID, buffer, packetSize, 0, (struct sockaddr *)&uName, &size);
#endif

    if (retVal != packetSize) {
        perror("in recvfrom");
        printf("called: %p, got back %d, %d\n", buffer, retVal, errno);
    }
    ASSERT(retVal == packetSize);
}

//----------------------------------------------------------------------
// SendToSocket
//      Transmit a fixed size packet to another Nachos' IPC port.
//      Abort on error.
//
//      For TCP/IP: sends to IP address and port based on machine ID
//      For Unix: sends to Unix domain socket file
//----------------------------------------------------------------------
void SendToSocket(int sockID, const char *buffer, int packetSize,
                  const char *toName) {
    int retVal;

#if USE_TCPIP_NETWORK
    // Extract destination machine ID from socket name (format: "SOCKET_N")
    int destMachine = atoi(toName + 7);  // Skip "SOCKET_" prefix

    if (destMachine < 0 || destMachine >= MAX_MACHINES) {
        printf("ERROR: Invalid destination machine %d\n", destMachine);
        ASSERT(false);
    }

    struct sockaddr_in destAddr;
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(BASE_PORT + destMachine);

    // Convert IP address string to binary form
    if (inet_pton(AF_INET, machineHosts[destMachine], &destAddr.sin_addr) <= 0) {
        printf("ERROR: Invalid IP address for machine %d: %s\n",
               destMachine, machineHosts[destMachine]);
        ASSERT(false);
    }

    retVal = sendto(sockID, buffer, packetSize, 0,
                    (struct sockaddr *)&destAddr, sizeof(destAddr));

    if (retVal != packetSize) {
        perror("sendto failed");
        printf("Failed to send to %s:%d (machine %d), sent %d/%d bytes\n",
               machineHosts[destMachine], BASE_PORT + destMachine, destMachine,
               retVal, packetSize);
    }
#else
    // Unix domain socket (original behavior)
    struct sockaddr_un uName;

    InitSocketName(&uName, toName);
    retVal = sendto(sockID, buffer, packetSize, 0, (sockaddr *)&uName,
                    sizeof(uName));
#endif

    ASSERT(retVal == packetSize);
}

//----------------------------------------------------------------------
// CallOnUserAbort
//      Arrange that "func" will be called when the user aborts (e.g., by
//      hitting ctl-C.
//----------------------------------------------------------------------

void CallOnUserAbort(VoidNoArgFunctionPtr func) {
    (void)signal(SIGINT, (VoidFunctionPtr)func);
}

//----------------------------------------------------------------------
// Sleep
//      Put the UNIX process running Nachos to sleep for x seconds,
//      to give the user time to start up another invocation of Nachos
//      in a different UNIX shell.
//----------------------------------------------------------------------

void Delay(int seconds) { (void)sleep((unsigned)seconds); }

//----------------------------------------------------------------------
// Abort
//      Quit and drop core.
//----------------------------------------------------------------------

void Abort() { abort(); }

//----------------------------------------------------------------------
// Exit
//      Quit without dropping core.
//----------------------------------------------------------------------

void Exit(int exitCode) { exit(exitCode); }

//----------------------------------------------------------------------
// RandomInit
//      Initialize the pseudo-random number generator.  We use the
//      now obsolete "srand" and "rand" because they are more portable!
//----------------------------------------------------------------------

void RandomInit(unsigned seed) { srand(seed); }

//----------------------------------------------------------------------
// Random
//      Return a pseudo-random number.
//----------------------------------------------------------------------

int Random() { return rand(); }

//----------------------------------------------------------------------
// AllocBoundedArray
//      Return an array, with the two pages just before
//      and after the array unmapped, to catch illegal references off
//      the end of the array.  Particularly useful for catching overflow
//      beyond fixed-size thread execution stacks.
//
//      Note: Just return the useful part!
//
//      "size" -- amount of useful space needed (in bytes)
//----------------------------------------------------------------------

char *AllocBoundedArray(int size) {
    int pgSize = getpagesize();
    char *ptr = new char[pgSize * 2 + size];

    mprotect(ptr, pgSize, 0);
    mprotect(ptr + pgSize + size, pgSize, 0);
    return ptr + pgSize;
}

//----------------------------------------------------------------------
// DeallocBoundedArray
//      Deallocate an array of integers, unprotecting its two boundary pages.
//
//      "ptr" -- the array to be deallocated
//      "size" -- amount of useful space in the array (in bytes)
//----------------------------------------------------------------------

void DeallocBoundedArray(char *ptr, int size) {
    int pgSize = getpagesize();

    mprotect(ptr - pgSize, pgSize, PROT_READ | PROT_WRITE | PROT_EXEC);
    mprotect(ptr + size, pgSize, PROT_READ | PROT_WRITE | PROT_EXEC);
    delete[] (ptr - pgSize);
}
