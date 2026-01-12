// thread.h
//      Data structures for managing threads.  A thread represents
//      sequential execution of code within a program.
//      So the state of a thread includes the program counter,
//      the processor registers, and the execution stack.
//
//      Note that because we allocate a fixed size stack for each
//      thread, it is possible to overflow the stack -- for instance,
//      by recursing to too deep a level.  The most common reason
//      for this occuring is allocating large data structures
//      on the stack.  For instance, this will cause problems:
//
//              void foo() { int buf[1000]; ...}
//
//      Instead, you should allocate all data structures dynamically:
//
//              void foo() { int *buf = new int[1000]; ...}
//
//
//      Bad things happen if you overflow the stack, and in the worst
//      case, the problem may not be caught explicitly.  Instead,
//      the only symptom may be bizarre segmentation faults.  (Of course,
//      other problems can cause seg faults, so that isn't a sure sign
//      that your thread stacks are too small.)
//
//      One thing to try if you find yourself with seg faults is to
//      increase the size of thread stack -- ThreadStackSize.
//
//      In this interface, forking a thread takes two steps.
//      We must first allocate a data structure for it: "t = new Thread".
//      Only then can we do the fork: "t->fork(f, arg)".
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef THREAD_H
#define THREAD_H

#include "copyright.h"
#include "synch.h"
#include "utility.h"
#include "system.h"
#include "nos_threads.h"

#ifdef USER_PROGRAM
#include "addrspace.h"
#include "machine.h"
#endif

// CPU register state to be saved on context switch.
// The SPARC and MIPS only need 10 registers, but the Snake needs 18.
// For simplicity, this is just the max over all architectures.
#define MachineStateSize 18

// Size of the thread's private execution stack.
// WATCH OUT IF THIS ISN'T BIG ENOUGH!!!!!
#define StackSize (4 * 1024) // in words

class Process;

// Thread state
enum ThreadStatus { JUST_CREATED, RUNNING, READY, BLOCKED, SLEEP, TERMINATED };

// external function, dummy routine whose sole job is to call Thread::Print
extern void ThreadPrint(int arg);

// The following class defines a "thread control block" -- which
// represents a single thread of execution.
//
//  Every thread has:
//     an execution stack for activation records ("stackTop" and "stack")
//     space to save CPU registers while not running ("machineState")
//     a "status" (running/ready/blocked)
//
//  Some threads also belong to a user address space; threads
//  that only run in the kernel have a NULL address space.

class Thread {

    friend Process;

    private:
        // NOTE: DO NOT CHANGE the order of these first two members.
        // THEY MUST be in this position for SWITCH to work.
        int* stackTop = nullptr;                 // the current stack pointer
        int machineState[MachineStateSize] = {}; // all registers except for stackTop

        const char* name = {};
        int* stack = nullptr; // Bottom of the stack. NULL if this is the main thread (If NULL, don't deallocate stack)
        ThreadStatus status = JUST_CREATED; // ready, running or blocked

        Process* process = nullptr;
        tid_t TID = 0; // The TID for this thread

        Thread* joiner = nullptr;
        Thread* join = nullptr;
        Semaphore sem;

        long long waitTime = 0; // Used to wake up sleeping threads

        unsigned char flags = 0;
        void* retval = nullptr;

        unsigned int userTlsBase = 0;
        unsigned int userStackBase = 0;
        unsigned int userStackLimit = 0;
        unsigned int userStackSize = 0;
        int* clearChildTid = nullptr;

        Thread(const char* debugName, Process* p, tid_t tid);

    public:
        Thread() = delete; // explicitly disable the default constructor
        ~Thread();                     // deallocate a Thread
        // NOTE -- thread being deleted
        // must not be running when delete
        // is called

        [[nodiscard]] const char* getName()const { return name; }
        [[nodiscard]] ThreadStatus getStatus() const { return status; }

        [[nodiscard]] Process* getProcess()const { return process; }
        [[nodiscard]] tid_t getTID() const { return TID; }

        [[nodiscard]] bool hasJoiner()const { return joiner != nullptr; }

        [[nodiscard]] long long getWaitTime() const { return waitTime; }

        [[nodiscard]] bool isDetached() const { return (flags & USER_THREAD_FLAG_DETACHED) != 0; }
        [[nodiscard]] bool isTerminated() const { return status == TERMINATED; }
        [[nodiscard]] void* getReturnValue() const { return retval; }

        [[nodiscard]] unsigned int getTlsBase() const { return userTlsBase; }
        [[nodiscard]] unsigned int getUserStackBase() const { return userStackBase; }
        [[nodiscard]] unsigned int getUserStackLimit() const { return userStackLimit; }
        [[nodiscard]] unsigned int getUserStackSize() const { return userStackSize; }
        [[nodiscard]] int* getClearChildTid() const { return clearChildTid; }

        [[nodiscard]] AddrSpace* getAddrSpace()const;

        void setJoiner(Thread* thread) {joiner = thread;}
        void setJoin(Thread* thread) {join = thread;}
        void setStatus(const ThreadStatus st) { status = st; }

        void setDetached(bool d);
        void setReturnValue(void* val) { retval = val; }

        void setTlsBase(const unsigned int addr) { userTlsBase = addr; }
        void setUserStack(const unsigned int base, const unsigned int size, const unsigned int limit) { userStackBase = base; userStackSize = size; userStackLimit = limit; }
        void setClearChildTid(int* addr) { clearChildTid = addr; }

        void clearUserStack() { userStackBase = 0; userStackSize = 0; userStackLimit = 0; }

        void InitUserContext(unsigned int entryPoint, unsigned int arg, unsigned int user_sp);

        // basic thread operations
        void Joiner();
        void Join();
        void Fork(VoidFunctionPtr func, int arg); // Make thread run (*func)(arg)
        void Yield();                             // Relinquish the CPU if any other thread is runnable
        void Sleep();                             // Put the thread to sleep and relinquish the processor
        void SleepUntil(long long tick);          // Sleep until specified tick
        void WakeUp();                            // Wake up the thread
        void Finish();                            // The thread is done executing
        void CheckOverflow();                     // Check if thread has overflowed its stack

        void Print() const { printf("%s, ", name); }

    private:
        // some of the private data for this class is listed above

        void StackAllocate(VoidFunctionPtr func, int arg);
        // Allocate a stack for thread.
        // Used internally by Fork()

#ifdef USER_PROGRAM
        // A thread running a user program actually has *two* sets of CPU registers
        // -- one for its state while executing user code, one for its state while
        // executing kernel code.

        int userRegisters[NumTotalRegs] = {}; // user-level CPU register state

    public:
        void SaveUserState();    // save user-level register state
        void RestoreUserState() const; // restore user-level register state
#endif
};

// Magical machine-dependent routines, defined in switch.s

extern "C" {
// First frame on thread execution stack;
//      enable interrupts
//      call "func"
//      (when func returns, if ever) call ThreadFinish()
void ThreadRoot();

// Stop running oldThread and start running newThread
void SWITCH(Thread* oldThread, Thread* newThread);
}

#endif // THREAD_H
