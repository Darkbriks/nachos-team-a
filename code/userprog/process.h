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

        //User owner; // TODO create User class for multiUser OS 

    public:
        ~Process();

        static Process * createProcess(OpenFile * executable);

        unsigned int getPId(){return PID;}
        AddrSpace * getSpace() const {return space;}
        Thread * getMainThread(){return mainThread;};
};

#endif // PROCESS_H
