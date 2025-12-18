#ifndef PROCESS_H
#define PROCESS_H

#include "copyright.h"
#include "system.h"

#define MAX_THREAD 10 // TODO check for user's max process
#define MAX_PROCESS 15 // TODO PUT it in User


class Process{
    private:
        Process(OpenFile * executable, char * return_code);

        static class BitMapThreadSafe *all_process; // TODO put it in USer one day ... I hope but I really don't know
                                                    
        class BitMap *all_threads;
        AddrSpace *space;
        unsigned int PID;
        Thread * mainThread;
        unsigned int threadNumber;
        Lock *threadNumberLock;
        Semaphore *threadExitSemaphore;

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
        void AddThread();

        /**
         * @brief Remove a thread from this address space
         */
        void RemoveThread();

        /**
         * @brief Wait for all threads of this address space to terminate
         */
        void WaitForAllThreadsTerminate();
};

#endif // PROCESS_H
