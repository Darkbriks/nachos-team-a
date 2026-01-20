#include "syscall.h"
#include "nos_errno.h"
#include "nos_stdio.h"

int main() {
		long long before_sleep;
		long long after_sleep;

		printf("=== Test Sleep syscall : simple sleep ===\n");
		GetCurrentTick(&before_sleep);
		Sleep(5000);
		GetCurrentTick(&after_sleep);
		printf("Main: Woke up after 5000 ticks\n");
		printf("Main: Slept for %d ticks (expected ~5000)\n", (int)(after_sleep - before_sleep));

		printf("=== Test Sleep syscall : long sleep ===\n");
		GetCurrentTick(&before_sleep);
		Sleep(50000);
		GetCurrentTick(&after_sleep);
		printf("Main: Woke up after 50000 ticks\n");
		printf("Main: Slept for %d ticks (expected ~50000)\n", (int)(after_sleep - before_sleep));

		printf("=== Test Sleep syscall : 0 tick sleep ===\n");
		GetCurrentTick(&before_sleep);
		Sleep(0);
		GetCurrentTick(&after_sleep);
		printf("Main: Woke up after 0 ticks\n");
		printf("Main: Slept for %d ticks (expected ~0)\n", (int)(after_sleep - before_sleep));

		printf("=== Test Sleep syscall : invalid sleep (negative ticks) ===\n");
		int errcode;
		GetCurrentTick(&before_sleep);
		int result = Sleep(-100);
		if (result == -1) {
				errcode = __get_errno();
				GetCurrentTick(&after_sleep);
				printf("Main: Sleep(-100) failed as expected with errno = %d\n", errcode);
		} else {
				GetCurrentTick(&after_sleep);
				printf("Main: Sleep(-100) unexpectedly succeeded\n");
		}

		printf("Main: Time after invalid sleep call\n");
		printf("Main: Slept for %d ticks (expected as low as possible)\n", (int)(after_sleep - before_sleep));

		return 0;
}
