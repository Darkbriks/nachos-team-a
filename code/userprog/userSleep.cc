#include "userSleep.h"
#include "exception.h"
#include "system.h"
#include "thread.h"
#include "nos_errno.h"
#include "process.h"

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

void handle_SC_Sleep() {
    int num_ticks = machine->ReadRegister(4);
    RETURN(do_UserSleep(num_ticks));
}

void handle_SC_SleepUntil() {
    const int tick = machine->ReadRegister(4);
    RETURN(do_UserSleepUntil(tick));
}

void handle_SC_GetCurrentTick() {
    const ptr_32 addr = machine->ReadRegister(4);

    Process *process = currentThread->getProcess();
    VALIDATE_ARG(process != nullptr, E_FAULT);

    const AddrSpace *space = process->getSpace();
    VALIDATE_ARG(space != nullptr, E_FAULT);

    VALIDATE_ARG(space->IsValidUserRange(addr, 8), E_FAULT);

    const long long current_tick = stats->totalTicks;
    const auto high = static_cast<int>(current_tick >> 32);
    const auto low = static_cast<int>(current_tick & 0xFFFFFFFF);

    if (!machine->WriteMem(addr, 4, low) || !machine->WriteMem(addr + 4, 4, high)) {
        RETURN(-E_FAULT);
    }
    RETURN(0);
}
