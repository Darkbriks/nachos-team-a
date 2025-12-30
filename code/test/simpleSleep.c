#include "syscall.h"
#include "nos_stdlib.h"

int main() {
		long long before_sleep;
		long long after_sleep;

		printf_simple("=== Test Sleep syscall : simple sleep ===\n");
		GetCurrentTick(&before_sleep);
		Sleep(5000);
		GetCurrentTick(&after_sleep);
		printf_simple("Main: Woke up after 5000 ticks\n");

		printf_simple("Main: Slept for ");
		PutInt(after_sleep - before_sleep);
		printf_simple(" ticks (expected ~5000)\n");

		printf_simple("=== Test Sleep syscall : long sleep ===\n");
		GetCurrentTick(&before_sleep);
		Sleep(50000);
		GetCurrentTick(&after_sleep);
		printf_simple("Main: Woke up after 50000 ticks\n");

		printf_simple("Main: Slept for ");
		PutInt(after_sleep - before_sleep);
		printf_simple(" ticks (expected ~50000)\n");

		printf_simple("=== Test Sleep syscall : 0 tick sleep ===\n");
		GetCurrentTick(&before_sleep);
		Sleep(0);
		GetCurrentTick(&after_sleep);
		printf_simple("Main: Woke up after 0 ticks\n");

		printf_simple("Main: Slept for ");
		PutInt(after_sleep - before_sleep);
		printf_simple(" ticks (expected ~0)\n");

		printf_simple("=== Test Sleep syscall : invalid sleep (negative ticks) ===\n");
		int errcode;
		GetCurrentTick(&before_sleep);
		int result = Sleep(-100);
		if (result == -1) {
				errcode = GetLastError();
				GetCurrentTick(&after_sleep);
				printf_simple("Main: Sleep(-100) failed as expected with errno = ");
				PutInt(errcode);
				printf_simple("\n");
		} else {
				GetCurrentTick(&after_sleep);
				printf_simple("Main: Sleep(-100) unexpectedly succeeded\n");
		}

		printf_simple("Main: Time after invalid sleep call\n");
		printf_simple("Main: Slept for ");;
		PutInt(after_sleep - before_sleep);
		printf_simple(" ticks (expected as low as possible)\n");

		return 0;
}