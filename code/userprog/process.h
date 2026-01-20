#ifndef PROCESS_H
#define PROCESS_H

#include "copyright.h"
#include "linked_list.h"
#include "system.h"
#include "thread.h"
#include "synch.h"

#define MAX_THREAD 30 // TODO check for user's max process
#define MAX_PROCESS 15

// TODO: One day, create User class for multiUser OS
//       Put MAX_PROCESS, all_process, all_process_lock, activeProcessCount, processCountLock in User class
//       And add a User* owner in Process class

class Lock;

class Process {
    friend class Thread;
    private:
        Process(inode_t inode, char* return_code);

        static class BitMap *all_process;
        static Lock* all_process_lock;
        static int activeProcessCount;
        static Lock *processCountLock;
        int exitCode;


        static LinkedList<Process>* all_process_addr;

        AddrSpace *space;
        unsigned int PID;
        Thread* mainThread;
        unsigned int threadNumber;
        Lock *threadNumberLock;
        Semaphore *threadExitSemaphore;
        Semaphore *ancestorSem;
        Process *ancestor;

        class BitMap* threads_bitmap;

        /**
         * @brief  Our global list of process in the machine
         */
        LinkedList<Thread>* all_threads_addr;

    public:
        ~Process();

        static bool isLastActiveProcess();
        static bool doesProcessExist(int PID);
        static unsigned int getCurrentNumberOfProcess();
        static Process* FindProcessByPID(const unsigned int PID); 

        static Process* createProcess(inode_t executable);

        [[nodiscard]] unsigned int getPId() const { return PID; }
        [[nodiscard]] AddrSpace* getSpace() const { return space; }
        [[nodiscard]] Thread* getMainThread() const { return mainThread; }
        [[nodiscard]] unsigned int GetThreadNumber() const { return threadNumber; }
        [[nodiscard]] Process* getAncestor() const { return ancestor; }
        [[nodiscard]] int getExitCode() { return exitCode; }
        void setExitCode(int code) { exitCode = code;}

        /**
         * @brief Add a thread for this address space
         */
        Thread* CreateThread(const char* name, ptr_32 tlsBase = 0);

        /**
         * @brief Mark a thread as terminated and decrease the thread count
         * The thread isn't deleted here, because it can be joined later
         */
        void ThreadTerminated(Thread* thread);

        /**
         * @brief Remove a thread from this address space
         */
        void RemoveThread(Thread* thread) const;

        /**
         * @brief Wait for all threads of this address space to terminate
         */
        void WaitForAllThreadsTerminate() const;

        /**
         * @brief Find the thread in process runinng thread
         *
         * @param TID the tid of the thread we search
         *
         * @result nullptr if the thread isn't find else a pointer on the thread
         */
        [[nodiscard]] Thread* FindThread(unsigned int TID) const;

        /**
         * @brief This function is use by the ancestor to wait his child
         */
        void AncestorWait();

        /**
         * @brief This function is use by the child to signal his ancestor he finish
         */
        void AncestorSigChild();

        /**
         * @brief This function is called for wait a child process
         *
         * @param child The pointer on the desired child to wait
         */
        void WaitForChild(Process* child);

        /**
         * @brief Delete all threads linked to a process
         */
        void KillAllThreads(bool include_current = true);

        /**
         * @brief This function is called only by Cleanup at the end of the program
         */
        static void freeAllStatic();
};

#endif // PROCESS_H
