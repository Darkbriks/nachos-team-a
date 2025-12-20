#include "syscall.h"
#include "my_stdlib.h"

int main() {
		long long before_sleep;
		long long after_sleep;

		my_printf("=== Test Sleep syscall : simple sleep ===\n");
		GetCurrentTick(&before_sleep);
		Sleep(5000);
		GetCurrentTick(&after_sleep);
		my_printf("Main: Woke up after 5000 ticks\n");

		my_printf("Main: Slept for ");
		PutInt(after_sleep - before_sleep);
		my_printf(" ticks (expected ~5000)\n");

		my_printf("=== Test Sleep syscall : long sleep ===\n");
		GetCurrentTick(&before_sleep);
		Sleep(50000);
		GetCurrentTick(&after_sleep);
		my_printf("Main: Woke up after 50000 ticks\n");

		my_printf("Main: Slept for ");
		PutInt(after_sleep - before_sleep);
		my_printf(" ticks (expected ~50000)\n");

		my_printf("=== Test Sleep syscall : 0 tick sleep ===\n");
		GetCurrentTick(&before_sleep);
		Sleep(0);
		GetCurrentTick(&after_sleep);
		my_printf("Main: Woke up after 0 ticks\n");

		my_printf("Main: Slept for ");
		PutInt(after_sleep - before_sleep);
		my_printf(" ticks (expected ~0)\n");

		my_printf("=== Test Sleep syscall : invalid sleep (negative ticks) ===\n");
		int errcode;
		GetCurrentTick(&before_sleep);
		int result = Sleep(-100);
		if (result == -1) {
				errcode = GetLastError();
				GetCurrentTick(&after_sleep);
				my_printf("Main: Sleep(-100) failed as expected with errno = ");
				PutInt(errcode);
				my_printf("\n");
		} else {
				GetCurrentTick(&after_sleep);
				my_printf("Main: Sleep(-100) unexpectedly succeeded\n");
		}

		my_printf("Main: Time after invalid sleep call\n");
		my_printf("Main: Slept for ");;
		PutInt(after_sleep - before_sleep);
		my_printf(" ticks (expected as low as possible)\n");

		return 0;
}