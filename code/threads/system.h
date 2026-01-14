// system.h
//      All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "interrupt.h"
#include "scheduler.h"
#include "stats.h"
#include "timer.h"
#include "utility.h"

class FutexQueue;
class FrameProvider;
class Process;
class Thread;

typedef int ptr_32;

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); // Initialization,
                                               // called before anything else
extern void Cleanup();                         // Cleanup, called when
                                               // Nachos is done.

extern FutexQueue* futexQueue;        // Global futex queue
extern FrameProvider* frameProvider;  // Physical memory frame provider
extern Thread *currentThread;         // the thread holding the CPU
extern Thread *threadToBeDestroyed;   // the thread that just finished
extern Process *processToBeDestroyed; // the process that just finished
extern Scheduler *scheduler;          // the ready list
extern Interrupt *interrupt;          // interrupt status
extern Statistics *stats;             // performance metrics
extern Timer *timer;                  // the hardware alarm clock

#define MAX_PATH_SIZE 1024

#ifdef USER_PROGRAM
#include "machine.h"
#include "synchconsole.h"

extern Machine *machine; // user program memory and registers
extern SynchConsole *synchConsole; // The only Console

bool CopyFromUserRaw(void* dst, unsigned src, size_t size);
bool CopyToUserRaw(unsigned dst, const void* src, size_t size);

bool CopyStringFromUser(unsigned src, char* dst, size_t max);
bool CopyStringToUser(const char* src, unsigned dst, size_t max);

template<typename T>
bool CopyFromUserType(T* dst, const unsigned src) {
    return CopyFromUserRaw(dst, src, sizeof(T));
}

template<typename T>
bool CopyToUserType(const unsigned dst, const T* src) {
    return CopyToUserRaw(dst, src, sizeof(T));
}
#endif

#ifdef FILESYS_NEEDED // FILESYS or FILESYS_STUB
#include "filesys.h"
extern FileSystem *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice *postOffice;
#endif

#endif // SYSTEM_H
