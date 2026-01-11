/* Start.s 
 *	Assembly language assist for user programs running on top of Nachos.
 *
 *	Since we don't want to pull in the entire C library, we define
 *	what we need for a user program here, namely Start and the system
 *	calls.
 */

#include "tls.h"
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

DEFINE_SIMPLE_SYSCALL PutChar, SC_PutChar
DEFINE_SYSCALL_STUB PutString, SC_PutString
DEFINE_SIMPLE_SYSCALL GetChar, SC_GetChar
DEFINE_SYSCALL_STUB GetString, SC_GetString
DEFINE_SYSCALL_STUB PutInt, SC_PutInt
DEFINE_SYSCALL_STUB GetInt, SC_GetInt

/* PthreadCreate is defined at the end of this file, because it needs
   a wrapper function for proper argument passing */
DEFINE_SIMPLE_SYSCALL PthreadExit, SC_PthreadExit
DEFINE_SYSCALL_STUB PthreadJoin, SC_PthreadJoin
DEFINE_SYSCALL_STUB PthreadDetach, SC_PthreadDetach

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

DEFINE_SYSCALL_STUB Sbrk, SC_Sbrk

DEFINE_SYSCALL_STUB SetTLS, SC_SetTLS
DEFINE_SIMPLE_SYSCALL GetTLS, SC_GetTLS

DEFINE_SYSCALL_STUB thread_create, SC_thread_create
DEFINE_SIMPLE_SYSCALL thread_exit, SC_thread_exit
DEFINE_SYSCALL_STUB thread_join, SC_thread_join
DEFINE_SIMPLE_SYSCALL thread_self, SC_thread_self
DEFINE_SIMPLE_SYSCALL thread_yield, SC_thread_yield

/* -------------------------------------------------------------
 * PthreadCreate wrapper
 * MIPS calling convention only allows 4 arguments in registers,
 * so we need a wrapper to pass the 5th argument on the stack.
 * -------------------------------------------------------------
 */
     .globl PthreadCreate
     .ent   PthreadCreate
 PthreadCreate:
     la    $8, PthreadExit_wrapper

     addiu $2,$0,SC_PthreadCreate
     syscall
     bgez  $2, 1f
     nop

     # Error
     subu  $3,$0,$2
     beqz  $gp, 2f
     nop
     sw    $3, TLS_OFFSET_ERRNO($gp)
     j     3f
     nop
 2:
     sw    $3, __global_errno
 3:
     addiu $2,$0,-1
     j     $31
     nop

 1:
     beqz  $gp, 4f
     nop
     sw    $0, TLS_OFFSET_ERRNO($gp)
     j     5f
     nop
 4:
     sw    $0, __global_errno
 5:
     j     $31
     nop
     .end PthreadCreate

/* -------------------------------------------------------------
 * Posix thread exit wrapper
 * -------------------------------------------------------------
 */
    .globl  PthreadExit_wrapper
    .ent    PthreadExit_wrapper
PthreadExit_wrapper:
    move   $4,$2
    j     PthreadExit
    nop
    .end    PthreadExit_wrapper

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
