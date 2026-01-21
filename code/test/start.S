/* Start.s 
 *	Assembly language assist for user programs running on top of Nachos.
 *
 *	Since we don't want to pull in the entire C library, we define
 *	what we need for a user program here, namely Start and the system
 *	calls.
 */

#include "nos_tls.h"
#include "syscall.h"
.include "syscall_macros.S"

        .text   
        .align  2

/* -------------------------------------------------------------
 * __start
 *	Initialize running a C program, by calling "main". 
 *
 * 	NOTE: This has to be first, so that it gets loaded at location 0.
 *	The Nachos kernel always starts a program by jumping to location 0.
 * -------------------------------------------------------------
 */

	.globl __start
	.ent	__start
__start:
    move    $gp,$0 /* initialize global pointer to 0 (no tls yet) */

	jal	main
	move	$4,$0		
	jal	Exit	 /* if we return from main, exit(0) */
	.end __start

/* -------------------------------------------------------------
 * System call stubs:
 *	Assembly language assist to make system calls to the Nachos kernel.
 *	There is one stub per system call, that places the code for the
 *	system call into register r2, and leaves the arguments to the
 *	system call alone (in other words, arg1 is in r4, arg2 is 
 *	in r5, arg3 is in r6, arg4 is in r7)
 *
 * 	The return value is in r2. This follows the standard C calling
 * 	convention on the MIPS.
 * -------------------------------------------------------------
 */

/* -------------------------------------------------------------
 * ERRNO HANDLING
 *
 * We support two modes:
 * 1. Global errno (when $gp == 0, TLS not initialized)
 * 2. Thread-local errno (when $gp != 0, points to TLS)
 *
 * ERRNO CONVENTION (as per NachOS documentation : https://im2ag-moodle.univ-grenoble-alpes.fr/pluginfile.php/5510/mod_resource/content/0/nachos-syscall.pdf):
 *   - If syscall returns negative value in $v0 (r2), it indicates an error
 *   - The stub saves abs(return value) in errno and returns -1
 *   - If syscall returns non-negative value, errno is set to 0
 *
 * Pattern:
 *   1. Execute syscall
 *   2. Check if $v0 (r2) >= 0
 *   3. If negative: errno = abs($v0), return -1
 *   4. If non-negative: errno = 0, return $v0
 * -------------------------------------------------------------
 */

DEFINE_SIMPLE_SYSCALL Halt, SC_Halt
DEFINE_SIMPLE_SYSCALL Exit, SC_Exit

DEFINE_SIMPLE_SYSCALL Create, SC_Create
DEFINE_SIMPLE_SYSCALL Open, SC_Open
DEFINE_SIMPLE_SYSCALL Read, SC_Read
DEFINE_SIMPLE_SYSCALL Write, SC_Write
DEFINE_SIMPLE_SYSCALL Close, SC_Close
DEFINE_SYSCALL_STUB FileLen, SC_FileLen
DEFINE_SYSCALL_STUB Seek, SC_Seek

DEFINE_SIMPLE_SYSCALL PutChar, SC_PutChar
DEFINE_SYSCALL_STUB PutString, SC_PutString
DEFINE_SIMPLE_SYSCALL GetChar, SC_GetChar
DEFINE_SYSCALL_STUB GetString, SC_GetString
DEFINE_SYSCALL_STUB PutInt, SC_PutInt
DEFINE_SYSCALL_STUB GetInt, SC_GetInt

DEFINE_SYSCALL_STUB Sleep, SC_Sleep
DEFINE_SYSCALL_STUB SleepUntil, SC_SleepUntil
DEFINE_SYSCALL_STUB GetCurrentTick, SC_GetCurrentTick

DEFINE_SYSCALL_STUB SemInit, SC_SemInit
DEFINE_SYSCALL_STUB SemWait, SC_SemWait
DEFINE_SYSCALL_STUB SemPost, SC_SemPost
DEFINE_SYSCALL_STUB SemDestroy, SC_SemDestroy
DEFINE_SYSCALL_STUB SetMaxSemForProcess, SC_SetMaxSemForProcess

DEFINE_SYSCALL_STUB ForkExec, SC_ForkExec
DEFINE_SYSCALL_STUB ForkJoin, SC_ForkJoin
DEFINE_SIMPLE_SYSCALL ForkSelf, SC_ForkSelf

DEFINE_SYSCALL_STUB Sbrk, SC_Sbrk
DEFINE_SYSCALL_STUB mmap, SC_mmap
DEFINE_SYSCALL_STUB munmap, SC_munmap

DEFINE_SYSCALL_STUB thread_create, SC_thread_create
DEFINE_SIMPLE_SYSCALL thread_exit, SC_thread_exit
DEFINE_SIMPLE_SYSCALL thread_self, SC_thread_self
DEFINE_SIMPLE_SYSCALL thread_yield, SC_thread_yield

DEFINE_SYSCALL_STUB futex_wait, SC_futex_wait
DEFINE_SYSCALL_STUB futex_wake, SC_futex_wake
DEFINE_SYSCALL_STUB atomic_cmpxchg, SC_atomic_cmpxchg
DEFINE_SYSCALL_STUB atomic_store, SC_atomic_store
DEFINE_SYSCALL_STUB atomic_load, SC_atomic_load

DEFINE_SYSCALL_STUB connect, SC_connect
DEFINE_SYSCALL_STUB listen, SC_listen
DEFINE_SYSCALL_STUB accept, SC_accept
DEFINE_SYSCALL_STUB sendto, SC_sendto
DEFINE_SYSCALL_STUB recvfrom, SC_recvfrom
DEFINE_SYSCALL_STUB close, SC_close

DEFINE_SYSCALL_STUB time, SC_time

/* dummy function to keep gcc happy */
        .globl  __main
        .ent    __main
__main:
        j       $31
        .end    __main

/* -------------------------------------------------------------
 * Data section - errno global variable
 * -------------------------------------------------------------
 */
	.data
	.align  2
	.globl  __global_errno
__global_errno:
	.word   0		/* errno initialized to 0 */
