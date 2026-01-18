// progtest.cc
//      Test routines for demonstrating that Nachos can load
//      a user program and execute it.
//
//      Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "addrspace.h"
#include "synchconsole.h"
#include "copyright.h"
#include "kernelpanic.h"
#include "synch.h"
#include "system.h"
#include "process.h"

//----------------------------------------------------------------------
// StartProcess
//      Run a user program.  Open the executable, load it into
//      memory, and jump to it.
//----------------------------------------------------------------------

void StartProcess(char *filename) {
    OpenFile* executable = fileSystem->Open(filename);

    if (executable == nullptr) {
        printf("Unable to open file %s\n", filename);
        interrupt->Halt();
        return;
    }

    const Process* newProcess = Process::createProcess(executable);
    if (newProcess == nullptr) {
        KERNEL_PANIC("StartProcess: Failed to create process");
    }

    const AddrSpace* space = newProcess->getSpace();
    ASSERT_KP(space != nullptr);
    space->InitRegisters();
    space->RestoreState();

    currentThread = newProcess->getMainThread();
    currentThread->setStatus(RUNNING);
    machine->Run(); // jump to the user progam
    KERNEL_PANIC("Returned from machine->Run() in StartProcess (should never happen)");
}

// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console* console;
static Semaphore* readAvail;
static Semaphore* writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
//      Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(int arg) { readAvail->V(); }

static void WriteDone(int arg) { writeDone->V(); }

//----------------------------------------------------------------------
// ConsoleTest
//      Test the console by echoing characters typed at the input onto
//      the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void ConsoleTest(char *in, char *out) {
    console = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);

    for (;;) {
        readAvail->P(); // wait for character to arrive
        const char ch = console->GetChar();
        if (ch == 'q' || ch == EOF) {
            delete readAvail;
            delete writeDone;
            return; // if q, quit
        }
        // if (ch == 'c'){
        //     console->PutChar('<'); // echo it!
        //     writeDone->P();       // wait for write to finish
        //     console->PutChar(ch); // echo it!
        //     writeDone->P();       // wait for write to finish
        //     console->PutChar('>'); // echo it!
        //     writeDone->P();       // wait for write to finish
        // } else {
        console->PutChar(ch); // echo it!
        writeDone->P();       // wait for write to finish
        // }
    }
}

//----------------------------------------------------------------------
// SynchConsoleTest
//      Test the SynchConsole by echoing characters typed at the input onto
//      the output.  Stop when the user types an EOF.
//----------------------------------------------------------------------
void SynchConsoleTest(char *in, char *out) {
    char ch;
    while( (ch = synchConsole->SynchGetChar()) != EOF){
        synchConsole->SynchPutChar(ch); // echo it!
    }
    fprintf(stderr, "Solaris: EOF detected in SynchConsole!\n");
}
