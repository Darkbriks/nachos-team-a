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
    // TODO: Fix this when step 4 is done
    const int stackAddr = static_cast<int>(currentThread->space->GetNumPages() * PageSize - 3 * PageSize);

    const int prevPC = machine->ReadRegister(PCReg);

    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();

    machine->WriteRegister(PCReg, param->get_function());
    machine->WriteRegister(NextPCReg, param->get_function() + 4);
    machine->WriteRegister(PrevPCReg, prevPC);
    machine->WriteRegister(StackReg, stackAddr);
    machine->WriteRegister(RetAddrReg, param->get_exit_addr());
    machine->WriteRegister(4, param->get_arg());

    delete param;
    machine->Run();
}

int do_UserThreadCreate(const int function, const int arg, const int exit_addr){
    Thread *thread = currentThread->space->getProcess()->CreateThread( (char *)("user_thread"));
    if (!thread){ return -E_NOMEM; }

    Param *param = new Param(function, arg, exit_addr);
    thread->Fork(StartUserThread, reinterpret_cast<int>(param));

    return static_cast<int>(thread->getTID());
}

void do_UserThreadExit(){
    currentThread->space->getProcess()->RemoveThread(currentThread);
    currentThread->Joiner();
    currentThread->Finish();
}

int do_UserThreadJoin(int TID){
    Thread * other_thread = currentThread->space->getProcess()->FindThread(TID);

    if (other_thread == nullptr){ return -E_NOSPC; }
    if (other_thread == currentThread){ return -E_INVAL; }
    if (other_thread->hasJoiner()){ return -E_INVAL; } // TODO: For now, only one joiner is allowed, discuss on what comportement we want

    currentThread->setJoin(other_thread);
    other_thread->setJoiner(currentThread);
    currentThread->Join();
    return 0;
}

int do_UserSleep(const int numTicks) {
    if (numTicks < 0) { return -E_INVAL; }
    if (numTicks == 0) { return 0; } // Sleep(0) is a no-op

    const IntStatus oldLevel = interrupt->SetLevel(IntOff);
    long long wakeTime = stats->totalTicks + numTicks;

    //Check for overflow
    if (wakeTime < stats->totalTicks) { interrupt->SetLevel(oldLevel); return -E_OVERFLOW; }

    currentThread->SleepUntil(wakeTime);
    interrupt->SetLevel(oldLevel);

    return 0;
}

int do_UserSleepUntil(long long tick) {
    if (tick < 0) { return -E_INVAL; }

    const IntStatus oldLevel = interrupt->SetLevel(IntOff);

    if (tick <= stats->totalTicks) { interrupt->SetLevel(oldLevel); return 0; }

    currentThread->SleepUntil(tick);
    interrupt->SetLevel(oldLevel);

    return 0;
}