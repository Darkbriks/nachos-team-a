#include "userthread.h"
#include "syscall.h"
#include "process.h"

/**
 * @brief appelée par le nouveau thread Nachos créé par la fonction do_UserThreadCreate
 *
 * @param f 
 */
/*
 * Cette fonction initialise les sauvegardes des registres d’une nouvelle copie de
l’interprète MIPS à la manière de l’interprète primitif (fonctions Machine::InitRegisters et
Machine::RestoreState) et lance l’interprète (Machine::Run).
Notez que vous aurez à initialiser le pointeur de pile. Il vous est suggéré de le placer 2 ou 3 pages en
dessous du pointeur du programme principal. Ceci est une évaluation empirique, bien sûr ! Il faudra
probablement faire mieux dans un deuxième temps...
*/
static void StartUserThread(const int f){
    const Param *param = reinterpret_cast<Param *>(f);

    // Initialize the stack pointer to 3 pages before the end of the address space
    // Probably needs to be adjusted later
    const int stackAddr = static_cast<int>(currentThread->space->GetNumPages() * PageSize - 3 * PageSize);

    const int prevPC = machine->ReadRegister(PCReg);

    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();

    machine->WriteRegister(PCReg, param->get_function());
    machine->WriteRegister(NextPCReg, param->get_function() + 4);
    machine->WriteRegister(PrevPCReg, prevPC);
    machine->WriteRegister(StackReg, stackAddr);
    // TODO Stackreg can be usefull to know when a thread lezve i's original function

    machine->WriteRegister(4, param->get_arg());

    delete param;
    machine->Run();
}

int do_UserThreadCreate(const int function, const int arg){
     
    Thread *thread = currentThread->space->getProcess()->CreateThread( (char *)("user_thread"));
    if (!thread){ return -E_NOMEM; }

    Param *param = new Param(function, arg);
    thread->Fork(StartUserThread, reinterpret_cast<int>(param));


    return static_cast<int>(thread->getTID());
}

void do_UserThreadExit(){
    currentThread->space->getProcess()->RemoveThread(currentThread);
    currentThread->Joiner();
    currentThread->Finish();
}

Thread *get_thread_by_TID(int TID){
    return nullptr;
}

int do_UserThreadJoin(int TID){
    currentThread->setJoin(get_thread_by_TID(TID));
    currentThread->Join();
    return 0;
}
