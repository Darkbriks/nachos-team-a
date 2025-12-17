#include "userthread.h"
#include "syscall.h"

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
static void StartUserThread(int f){
    printf("coucou\n");
    Param *param = (Param *) f;
    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();
    machine->WriteRegister(PCReg, param->get_function());
    machine->WriteMem(machine->ReadRegister(StackReg) + 4, 4, param->get_arg());
    printf("coucou\n");
    machine->Run();
    printf("coucou\n");
    delete param;
}

int do_UserThreadCreate(int function, int arg){
    Thread *thread = new Thread("coucou");
    if (!thread){
        return -E_NOMEM;
    }

    Param *param = new Param(function, arg);
    thread->Fork(StartUserThread, (int) param);
    return thread->getTID();
}

void do_UserThreadExit(){
    // interrupt->SetLevel(IntOff);
    Thread *oldThread = currentThread;
    oldThread->Yield();
    oldThread->Finish(); 
}
