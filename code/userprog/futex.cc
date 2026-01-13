#include "futex.h"

#include "addrspace.h"
#include "exception.h"
#include "process.h"
#include "syscall.h"
#include "system.h"
#include "../machine/interrupt.h"

FutexQueue::FutexQueue() = default;

FutexQueue::~FutexQueue() {
    for (auto&[key, val] : futex_map) {
        delete val;
    }
}

int FutexQueue::wait(const int uaddr, const int expected) {
    const IntStatus oldLevel = interrupt->SetLevel(IntOff);

    int val;
    if (CopyFromUserType<int>(&val, uaddr) == false) {
        DEBUG('y', "Futex wait: invalid user address %d\n", uaddr);
        interrupt->SetLevel(oldLevel);
        return E_FAULT;
    }

    if (val != expected) {
        DEBUG('y', "Futex wait: value mismatch at address %d (expected %d, got %d)\n", uaddr, expected, val);
        interrupt->SetLevel(oldLevel);
        return E_AGAIN;
    }

    FutexWaiter* futex_waiter;
    if (const auto it = futex_map.find(uaddr); it == futex_map.end()) {
        futex_waiter = new FutexWaiter(uaddr);
        futex_map[uaddr] = futex_waiter;
    } else {
        futex_waiter = it->second;
    }

    futex_waiter->add_waiter(currentThread);

    currentThread->Sleep();

    interrupt->SetLevel(oldLevel);
    return 0;
}

int FutexQueue::wake(const int uaddr, const int num_wake) {
    const IntStatus oldLevel = interrupt->SetLevel(IntOff);

    const auto it = futex_map.find(uaddr);
    if (it == futex_map.end()) {
        interrupt->SetLevel(oldLevel);
        return 0; // No waiters
    }

    FutexWaiter* futex_waiter = it->second;
    int woken_count = 0;

    for (int i = 0; i < num_wake; ++i) {
        if (!futex_waiter->has_waiters()) { break; }
        Thread* thread = futex_waiter->remove_waiter();
        thread->WakeUp();
        ++woken_count;
    }

    if (!futex_waiter->has_waiters()) {
        delete futex_waiter;
        futex_map.erase(it);
    }

    interrupt->SetLevel(oldLevel);
    return woken_count;
}

void handle_SC_futex_wait() {
    const int uaddr = machine->ReadRegister(4);
    const int expected = machine->ReadRegister(5);

    const Process* process = currentThread->getProcess();
    VALIDATE_ARG(process != nullptr, -E_FAULT);

    const AddrSpace* space = process->getSpace();
    VALIDATE_ARG(space != nullptr, -E_FAULT);

    VALIDATE_ARG(space->IsUserAddress(uaddr), -E_FAULT);

    RETURN(-futexQueue->wait(uaddr, expected));
}

void handle_SC_futex_wake() {
    const int uaddr = machine->ReadRegister(4);
    const int num_wake = machine->ReadRegister(5);

    const Process* process = currentThread->getProcess();
    VALIDATE_ARG(process != nullptr, -E_FAULT);

    const AddrSpace* space = process->getSpace();
    VALIDATE_ARG(space != nullptr, -E_FAULT);

    VALIDATE_ARG(space->IsUserAddress(uaddr), -E_FAULT);

    RETURN(futexQueue->wake(uaddr, num_wake));
}

void handle_SC_atomic_cmpxchg() {
    const int uaddr = machine->ReadRegister(4);
    const int expected = machine->ReadRegister(5);
    const int new_value = machine->ReadRegister(6);

    const Process* process = currentThread->getProcess();
    VALIDATE_ARG(process != nullptr, -E_FAULT);

    const AddrSpace* space = process->getSpace();
    VALIDATE_ARG(space != nullptr, -E_FAULT);

    VALIDATE_ARG(space->IsUserAddress(uaddr), -E_FAULT);

    const IntStatus oldLevel = interrupt->SetLevel(IntOff);

    int current_value;
    if (CopyFromUserType<int>(&current_value, uaddr) == false) {
        DEBUG('y', "Atomic cmpxchg: invalid user address %d\n", uaddr);
        interrupt->SetLevel(oldLevel);
        RETURN(-E_FAULT);
    }

    if (current_value == expected) {
        if (CopyToUserType<int>(uaddr, &new_value) == false) {
            DEBUG('y', "Atomic cmpxchg: invalid user address %d\n", uaddr);
            interrupt->SetLevel(oldLevel);
            RETURN(-E_FAULT);
        }
    }

    interrupt->SetLevel(oldLevel);
    RETURN(current_value);
}

void handle_SC_atomic_store() {
    const int uaddr = machine->ReadRegister(4);
    const int value = machine->ReadRegister(5);

    const Process* process = currentThread->getProcess();
    VALIDATE_ARG(process != nullptr, -E_FAULT);

    const AddrSpace* space = process->getSpace();
    VALIDATE_ARG(space != nullptr, -E_FAULT);

    VALIDATE_ARG(space->IsUserAddress(uaddr), -E_FAULT);

    const IntStatus oldLevel = interrupt->SetLevel(IntOff);

    if (CopyToUserType<int>(uaddr, &value) == false) {
        DEBUG('y', "Atomic store: invalid user address %d\n", uaddr);
        interrupt->SetLevel(oldLevel);
        RETURN(-E_FAULT);
    }

    interrupt->SetLevel(oldLevel);
    RETURN(0);
}

void handle_SC_atomic_load() {
    const int uaddr = machine->ReadRegister(4);

    const Process* process = currentThread->getProcess();
    VALIDATE_ARG(process != nullptr, -E_FAULT);

    const AddrSpace* space = process->getSpace();
    VALIDATE_ARG(space != nullptr, -E_FAULT);

    VALIDATE_ARG(space->IsUserAddress(uaddr), -E_FAULT);

    const IntStatus oldLevel = interrupt->SetLevel(IntOff);

    int value;
    if (CopyFromUserType<int>(&value, uaddr) == false) {
        DEBUG('y', "Atomic load: invalid user address %d\n", uaddr);
        interrupt->SetLevel(oldLevel);
        RETURN(-E_FAULT);
    }

    interrupt->SetLevel(oldLevel);
    RETURN(value);
}