// scheduler.cc
//      Routines to choose the next thread to run, and to dispatch to
//      that thread.
//
//      These routines assume that interrupts are already disabled.
//      If interrupts are disabled, we can assume mutual exclusion
//      (since we are on a uniprocessor).
//
//      NOTE: We can't use Locks to provide mutual exclusion here, since
//      if we needed to wait for a lock, and the lock was busy, we would
//      end up calling FindNextToRun(), and that would put us in an
//      infinite loop.
//
//      Very simple implementation -- no priorities, straight FIFO.
//      Might need to be improved in later assignments.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "scheduler.h"
#include "copyright.h"
#include "process.h"
#include "system.h"
#include "thread.h"
#include "kernelpanic.h"

//----------------------------------------------------------------------
// Scheduler::Scheduler
//      Initialize the list of ready but not running threads to empty.
//----------------------------------------------------------------------

Scheduler::Scheduler() { readyList = new List; sleepList = new List; }

//----------------------------------------------------------------------
// Scheduler::~Scheduler
//      De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler() { delete readyList; delete sleepList; }

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
//      Mark a thread as ready, but not running.
//      Put it on the ready list, for later scheduling onto the CPU.
//
//      "thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

void Scheduler::ReadyToRun(Thread *thread) {
    DEBUG('S', "Putting thread %s on ready list.\n", thread->getName());

    thread->setStatus(READY);
    readyList->Append((void *)thread);
}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
//      Return the next thread to be scheduled onto the CPU.
//      If there are no ready threads, return NULL.
// Side effect:
//      Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *Scheduler::FindNextToRun() { return (Thread *)readyList->Remove(); }

//----------------------------------------------------------------------
// Scheduler::Run
//      Dispatch the CPU to nextThread.  Save the state of the old thread,
//      and load the state of the new thread, by calling the machine
//      dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//      already been changed from running to blocked or ready (depending).
// Side effect:
//      The global variable currentThread becomes nextThread.
//
//      "nextThread" is the thread to be put into the CPU.
//----------------------------------------------------------------------

void Scheduler::Run(Thread *nextThread) {
    Thread *oldThread = currentThread;

    // LB: For safety...
    ASSERT_KP(interrupt->getLevel() == IntOff);
    // End of addition

#ifdef USER_PROGRAM                     // ignore until running user programs
    if (currentThread->getAddrSpace() != NULL) { // if this thread is a user program,
        currentThread->SaveUserState(); // save the user's CPU registers
        currentThread->getAddrSpace()->SaveState();
    }
#endif

    oldThread->CheckOverflow(); // check if the old thread
    // had an undetected stack overflow

    currentThread = nextThread;        // switch to the next thread
    currentThread->setStatus(RUNNING); // nextThread is now running

    DEBUG('S', "Switching from thread \"%s\" to thread \"%s\"\n",
          oldThread->getName(), nextThread->getName());

    // This is a machine-dependent assembly language routine defined
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".

    SWITCH(oldThread, nextThread);

    DEBUG('S', "Now in thread \"%s\"\n", currentThread->getName());

    // If the old thread gave up the processor because it was finishing,
    // we need to delete its carcass.  Note we cannot delete the thread
    // before now (for example, in Thread::Finish()), because up to this
    // point, we were still running on the old thread's stack!
    if (threadToBeDestroyed != NULL) {
        delete threadToBeDestroyed;
        threadToBeDestroyed = NULL;
    }

    if (processToBeDestroyed != NULL) {
        delete processToBeDestroyed;
        processToBeDestroyed = NULL;
    }

#ifdef USER_PROGRAM
    if (currentThread->getAddrSpace() != NULL) {    // if there is an address space
        currentThread->RestoreUserState(); // to restore, do it.
        currentThread->getAddrSpace()->RestoreState();
    }
#endif
}

//----------------------------------------------------------------------
// Scheduler::AddToSleepList
//      Add a thread to the sleep list
//----------------------------------------------------------------------

void Scheduler::AddToSleepList(Thread *thread) {
    DEBUG('S', "Adding thread %s to sleep list (wake time: %lld)\n", thread->getName(), thread->getWaitTime());
    thread->setStatus(BLOCKED);
    sleepList->SortedInsert(thread, thread->getWaitTime());
}

//----------------------------------------------------------------------
// Scheduler::WakeUpThreads
//      Wake up threads whose sleep time has expired
//----------------------------------------------------------------------

void Scheduler::WakeUpThreads() {
    const long long currentTick = stats->totalTicks;
    while (!sleepList->IsEmpty()) {
        auto *thread = static_cast<Thread *>(sleepList->GetFirst());

        if (thread->getWaitTime() > currentTick) { break; } // No more threads to wake up

        long long key;
        sleepList->SortedRemove(&key);

        DEBUG('S', "Waking up thread %s (wake time: %lld, current tick: %lld)\n", thread->getName(), thread->getWaitTime(), currentTick);
        ReadyToRun(thread);
    }
}

//----------------------------------------------------------------------
// Scheduler::Print
//      Print the scheduler state -- in other words, the contents of
//      the ready list.  For debugging.
//----------------------------------------------------------------------
void Scheduler::Print() {
    printf("Ready list contents:\n");
    readyList->Mapcar((VoidFunctionPtr)ThreadPrint);
}
