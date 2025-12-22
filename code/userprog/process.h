#ifndef PROCESS_H
#define PROCESS_H

#include "copyright.h"
#include "linked_list.h"
#include "system.h"

#define MAX_THREAD 10 // TODO check for user's max process
#define MAX_PROCESS 15 // TODO PUT it in User


class Process{
    friend class Thread;
    private:
        Process(OpenFile * executable, char * return_code);

        static class BitMapThreadSafe *all_process; // TODO put it in USer one day ... I hope but I really don't know

        AddrSpace *space;
        unsigned int PID;
        Thread * mainThread;
        unsigned int threadNumber;
        Lock *threadNumberLock;
        Semaphore *threadExitSemaphore;

        class BitMap *threads_bitmap;
        LinkedList<Thread> * all_threads_addr;

        //User owner; // TODO create User class for multiUser OS 

    public:
        ~Process();

        static Process * createProcess(OpenFile * executable);

        unsigned int getPId(){return PID;}
        AddrSpace * getSpace() const {return space;}
        Thread * getMainThread(){return mainThread;};
        unsigned int GetThreadNumber() const { return threadNumber; }
        /**
         * @brief Add a thread for this address space
         */
        Thread* CreateThread(char * name);

        /**
         * @brief Mark a thread as terminated and decrease the thread count
         * The thread isn't deleted here, because it can be joined later
         */
        void ThreadTerminated(Thread * thread);

        /**
         * @brief Remove a thread from this address space
         */
        void RemoveThread(Thread * thread);

        /**
         * @brief Wait for all threads of this address space to terminate
         */
        void WaitForAllThreadsTerminate();

        /**
         * @brief Find the thread in process runinng thread
         *
         * @param TID the tid of the thread we search
         *
         * @result nullptr if the thread isn't find else a pointer on the thread
         */
        Thread * FindThread(unsigned int TID);
};

#endif // PROCESS_H
