#ifndef KERNEL_PANIC_H
#define KERNEL_PANIC_H

#include "machine.h"
#include "thread.h"
#include "process.h"
#include "stats.h"
#include "scheduler.h"
#include "frameprovider.h"
#include <stdio.h>

inline void KernelDumpState(const char* file, const int line, const char* msg) {
    fprintf(stderr, "\n");
    fprintf(stderr, "================================================================================\n");
    fprintf(stderr, "                            KERNEL PANIC\n");
    fprintf(stderr, "================================================================================\n");
    fprintf(stderr, "Location: %s:%d\n", file, line);
    fprintf(stderr, "Message:  %s\n", msg);
    fprintf(stderr, "================================================================================\n\n");

    fprintf(stderr, "[SYSTEM STATE]\n");
    fprintf(stderr, "  Total Ticks: %lld\n", stats->totalTicks);
    fprintf(stderr, "  User Ticks:  %lld\n", stats->userTicks);
    fprintf(stderr, "  System Ticks: %lld\n", stats->systemTicks);
    fprintf(stderr, "  Idle Ticks:  %lld\n", stats->idleTicks);
    fprintf(stderr, "\n");

    if (currentThread != nullptr) {
        fprintf(stderr, "[CURRENT THREAD]\n");
        fprintf(stderr, "  Name:   %s\n", currentThread->getName());
        fprintf(stderr, "  TID:    %u\n", currentThread->getTID());
        fprintf(stderr, "  Status: ");
        switch (currentThread->getStatus()) {
            case JUST_CREATED: fprintf(stderr, "JUST_CREATED\n"); break;
            case RUNNING:      fprintf(stderr, "RUNNING\n"); break;
            case READY:        fprintf(stderr, "READY\n"); break;
            case BLOCKED:      fprintf(stderr, "BLOCKED\n"); break;
            case SLEEP:        fprintf(stderr, "SLEEP\n"); break;
            case TERMINATED:   fprintf(stderr, "TERMINATED\n"); break;
            default:           fprintf(stderr, "UNKNOWN(%d)\n", currentThread->getStatus());
        }

        Process* process = currentThread->getProcess();
        if (process != nullptr) {
            fprintf(stderr, "  Process PID: %u\n", process->getPId());
            fprintf(stderr, "  Thread Count: %u\n", process->GetThreadNumber());
        } else {
            fprintf(stderr, "  Process: NULL\n");
        }
        fprintf(stderr, "\n");
    } else {
        fprintf(stderr, "[CURRENT THREAD]\n");
        fprintf(stderr, "  NULL\n\n");
    }

#ifdef USER_PROGRAM
    if (machine != nullptr) {
        fprintf(stderr, "[CPU REGISTERS]\n");
        fprintf(stderr, "  PC:       0x%08x\n", machine->ReadRegister(PCReg));
        fprintf(stderr, "  NextPC:   0x%08x\n", machine->ReadRegister(NextPCReg));
        fprintf(stderr, "  PrevPC:   0x%08x\n", machine->ReadRegister(PrevPCReg));
        fprintf(stderr, "  SP:       0x%08x\n", machine->ReadRegister(StackReg));
        fprintf(stderr, "  RetAddr:  0x%08x\n", machine->ReadRegister(RetAddrReg));

        fprintf(stderr, "\n  General Purpose Registers:\n");
        for (int i = 0; i < NumGPRegs; i += 4) {
            fprintf(stderr, "    r%-2d: 0x%08x  r%-2d: 0x%08x  r%-2d: 0x%08x  r%-2d: 0x%08x\n",
                    i,     machine->ReadRegister(i),
                    i + 1, machine->ReadRegister(i + 1),
                    i + 2, machine->ReadRegister(i + 2),
                    i + 3, machine->ReadRegister(i + 3));
        }
        fprintf(stderr, "  HI:       0x%08x\n", machine->ReadRegister(HiReg));
        fprintf(stderr, "  LO:       0x%08x\n", machine->ReadRegister(LoReg));
        fprintf(stderr, "\n");

        if (machine->pageTable != nullptr && machine->pageTableSize > 0) {
            fprintf(stderr, "[PAGE TABLE] (size: %u)\n", machine->pageTableSize);
            unsigned int validPages = 0;
            unsigned int dirtyPages = 0;
            for (unsigned int i = 0; i < machine->pageTableSize; i++) {
                if (machine->pageTable[i].valid) {
                    validPages++;
                    if (machine->pageTable[i].dirty) dirtyPages++;
                }
            }
            fprintf(stderr, "  Valid pages: %u/%u\n", validPages, machine->pageTableSize);
            fprintf(stderr, "  Dirty pages: %u\n", dirtyPages);

            fprintf(stderr, "  First valid entries:\n");
            for (unsigned int i = 0; i < machine->pageTableSize; i++) {
                if (machine->pageTable[i].valid) {
                    fprintf(stderr, "    [%3u] virt=0x%08x -> phys=%3d %s%s%s\n",
                            i,
                            i * PageSize,
                            machine->pageTable[i].physicalPage,
                            machine->pageTable[i].readOnly ? "RO " : "RW ",
                            machine->pageTable[i].use ? "U " : "- ",
                            machine->pageTable[i].dirty ? "D" : "-");
                }
            }
            fprintf(stderr, "\n");
        }

        if (frameProvider != nullptr) {
            fprintf(stderr, "[MEMORY]\n");
            fprintf(stderr, "  Available frames: %d\n", frameProvider->NumAvailFrame());
            fprintf(stderr, "  Total frames:     %d\n", NumPhysPages);
            fprintf(stderr, "\n");
        }
    }
#endif

    if (scheduler != nullptr) {
        fprintf(stderr, "[SCHEDULER]\n");
        fprintf(stderr, "  Ready queue: ");
        scheduler->Print();
        fprintf(stderr, "\n");
    }

    fprintf(stderr, "[PROCESSES]\n");
    fprintf(stderr, "  Active process count: %u\n", Process::getCurrentNumberOfProcess());
    fprintf(stderr, "\n");

    fprintf(stderr, "================================================================================\n");
    fprintf(stderr, "                         HALTING SYSTEM\n");
    fprintf(stderr, "================================================================================\n\n");
}

#define KERNEL_PANIC(msg) do { \
    KernelDumpState(__FILE__, __LINE__, msg); \
    interrupt->Halt(); \
} while(0)

#define KERNEL_PANIC_ON_ASSERT(cond) do { \
    KernelDumpState(__FILE__, __LINE__, "Assertion failure: " #cond); \
    Abort(); \
} while(0)

#endif