
#include "userSleep.h"
#include "exception.h"
#include "system.h"
#include "thread.h"
#include "syscall.h"


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

    if (addr < 0) { RETURN(-E_FAULT); } // TODO Add more checks (addr is in valid user space, for example)

    const long long current_tick = stats->totalTicks;
    const auto high = static_cast<int>(current_tick >> 32);
    const auto low = static_cast<int>(current_tick & 0xFFFFFFFF);

    DEBUG('a', "GetCurrentTick: current_tick=%lld, high=0x%x, low=0x%x\n", current_tick, high, low);

    if (!machine->WriteMem(addr, 4, low) || !machine->WriteMem(addr + 4, 4, high)) {
        RETURN(-E_FAULT);
    }
    RETURN(0);
}
