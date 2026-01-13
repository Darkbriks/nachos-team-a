#ifndef FUTEX_H
#define FUTEX_H

#include "../threads/list.h"
#include "../threads/thread.h"
#include <unordered_map>

class FutexQueue {
public:
    FutexQueue();
    ~FutexQueue();

    int wait(int uaddr, int expected);
    int wake(int uaddr, int num_wake);

private:
    class FutexWaiter {
        struct FutexKey {
            Process* process;
            int uaddr;

            bool operator==(const FutexKey& other) const {
                return process == other.process && uaddr == other.uaddr;
            }
        };

        FutexKey key;
        List waiters;
    public:
        explicit FutexWaiter(const int addr) : key{currentThread->getProcess(), addr} {}

        [[nodiscard]] FutexKey get_key() const { return key; }
        void add_waiter(Thread* thread) { waiters.Append(thread); }
        Thread* remove_waiter() { return static_cast<Thread*>(waiters.Remove()); }
        [[nodiscard]] bool has_waiters() const { return !waiters.IsEmpty(); }
    };

    std::unordered_map<int, FutexWaiter*> futex_map; // TODO: Don't use std::map
};

void handle_SC_futex_wait();
void handle_SC_futex_wake();
void handle_SC_atomic_cmpxchg();
void handle_SC_atomic_store();
void handle_SC_atomic_load();

#endif