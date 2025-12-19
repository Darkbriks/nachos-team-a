/* Start.s 
 *	Assembly language assist for user programs running on top of Nachos.
 *
 *	Since we don't want to pull in the entire C library, we define
 *	what we need for a user program here, namely Start and the system
 *	calls.
 */

#include "syscall.h"

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
 * ERRNO CONVENTION (as per NachOS documentation : https://im2ag-moodle.univ-grenoble-alpes.fr/pluginfile.php/5510/mod_resource/content/0/nachos-syscall.pdf):
 *   - If syscall returns negative value in $v0 (r2), it indicates an error
 *   - The stub saves abs(return value) in errno and returns -1
 *   - If syscall returns non-negative value, errno is set to 0
 * -------------------------------------------------------------
 */

/* -------------------------------------------------------------
 * Syscalls with errno handling (syscalls that can fail)
 *
 * Pattern:
 *   1. Execute syscall
 *   2. Check if $v0 (r2) >= 0
 *   3. If negative: errno = abs($v0), return -1
 *   4. If non-negative: errno = 0, return $v0
 * -------------------------------------------------------------
 */
 
/* -------------------------------------------------------------
 * Example syscall with error handling:
 *      .globl SYSCALL
 *      .ent	SYSCALL
 *  SYSCALL:
 *      addiu $2,$0,SC_SYSCALL
 *      syscall
 *
 *      bgez  $2,SYSCALL_ok   # Jump if no error
 *      nop                   # In MIPS, instruction after branch is always executed, so we put a NOP here
 *
 *      # Error handling
 *      subu   $3,$0,$2       # $3 = -$2 (abs value of error)
 *      sw     $3,errno       # errno = abs(return value)
 *      addiu  $2,$0,-1       # return -1
 *      j      $31            # Return
 *      nop                   # NOP after jump
 *  SYSCALL_ok:
 *      sw     $0,errno       # errno = 0
 *      j      $31            # Return
 *      .end SYSCALL
 * -------------------------------------------------------------
 */

/* Halt : No error handling */
	.globl Halt
	.ent	Halt
Halt:
	addiu $2,$0,SC_Halt
	syscall
	j	$31
	.end Halt

/* Exit : No error handling */
	.globl Exit
	.ent	Exit
Exit:
	addiu $2,$0,SC_Exit
	syscall
	j	$31
	.end Exit

/* Exec : No error handling */
	.globl Exec
	.ent	Exec
Exec:
	addiu $2,$0,SC_Exec
	syscall
	j	$31
	.end Exec

/* Join : No error handling */
	.globl Join
	.ent	Join
Join:
	addiu $2,$0,SC_Join
	syscall
	j	$31
	.end Join

/* Create : No error handling */
	.globl Create
	.ent	Create
Create:
	addiu $2,$0,SC_Create
	syscall
	j	$31
	.end Create

/* Open : No error handling */
	.globl Open
	.ent	Open
Open:
	addiu $2,$0,SC_Open
	syscall
	j	$31
	.end Open

/* Read : No error handling */
	.globl Read
	.ent	Read
Read:
	addiu $2,$0,SC_Read
	syscall
	j	$31
	.end Read

/* Write : No error handling */
	.globl Write
	.ent	Write
Write:
	addiu $2,$0,SC_Write
	syscall
	j	$31
	.end Write

/* Close : No error handling */
	.globl Close
	.ent	Close
Close:
	addiu $2,$0,SC_Close
	syscall
	j	$31
	.end Close

/* Fork : No error handling */
	.globl Fork
	.ent	Fork
Fork:
	addiu $2,$0,SC_Fork
	syscall
	j	$31
	.end Fork

/* Yield : No error handling */
	.globl Yield
	.ent	Yield
Yield:
	addiu $2,$0,SC_Yield
	syscall
	j	$31
	.end Yield

/* PutChar : no error handling */
	.globl PutChar
    .ent	PutChar
PutChar:
    addiu $2,$0,SC_PutChar
    syscall
    j      $31
    .end PutChar

/* PutString : with error handling */
    .globl PutString
    .ent	PutString
PutString:
    addiu $2,$0,SC_PutString
    syscall
    bgez  $2,PutString_ok
    nop
    # Error handling
    subu   $3,$0,$2
    sw     $3,errno
    addiu  $2,$0,-1
    j      $31
    nop
PutString_ok:
    sw     $0,errno
    j      $31
    .end PutString

/* GetChar : no error handling */
	.globl GetChar
    .ent	GetChar
GetChar:
    addiu $2,$0,SC_GetChar
    syscall
    j      $31
    .end GetChar

/* GetString : with error handling */
    .globl GetString
    .ent	GetString
GetString:
    addiu $2,$0,SC_GetString
    syscall
    bgez  $2,GetString_ok
    nop
    # Error handling
    subu   $3,$0,$2
    sw     $3,errno
    addiu  $2,$0,-1
    j      $31
    nop
GetString_ok:
    sw     $0,errno
    j      $31
    .end GetString

/* PutInt : with error handling */
    .globl PutInt
    .ent	PutInt
PutInt:
    addiu $2,$0,SC_PutInt
    syscall
    bgez  $2,PutInt_ok
    nop
    # Error handling
    subu   $3,$0,$2
    sw     $3,errno
    addiu  $2,$0,-1
    j      $31
    nop
PutInt_ok:
    sw     $0,errno
    j      $31
    .end PutInt

/* GetInt : with error handling */
    .globl GetInt
    .ent	GetInt
GetInt:
    addiu $2,$0,SC_GetInt
    syscall
    bgez  $2,GetInt_ok
    nop
    # Error handling
    subu   $3,$0,$2
    sw     $3,errno
    addiu  $2,$0,-1
    j      $31
    nop
GetInt_ok:
    sw     $0,errno
    j      $31
    .end GetInt

/* CreateThread: with error handling */
    .globl CreateThread
    .ent	CreateThread
CreateThread:
    addiu $2,$0,SC_CreateThread
    syscall
    bgez  $2,CreateThread_ok
    nop
    # Error handling
    subu   $3,$0,$2
    sw     $3,errno
    addiu  $2,$0,-1
    j      $31
    nop
CreateThread_ok:
    sw     $0,errno
    j      $31
    .end CreateThread

/* ExitThread : with error handling */
    .globl ExitThread
    .ent	ExitThread
ExitThread:
    addiu $2,$0,SC_ExitThread
    syscall
    bgez  $2,ExitThread_ok
    nop
    # Error handling
    subu   $3,$0,$2
    sw     $3,errno
    addiu  $2,$0,-1
    j      $31
    nop
ExitThread_ok:
    sw     $0,errno
    j      $31
    .end ExitThread

/* JoinThread : with error handling */
    .globl JoinThread
    .ent	JoinThread
JoinThread:
    addiu $2,$0,SC_JoinThread
    syscall
    bgez  $2,JoinThread_ok
    nop
    # Error handling
    subu   $3,$0,$2
    sw     $3,errno
    addiu  $2,$0,-1
    j      $31
    nop
JoinThread_ok:
    sw     $0,errno
    j      $31
    .end JoinThread
    
/* Sleep : with error handling */
    .globl Sleep
    .ent	Sleep
Sleep:
    addiu $2,$0,SC_Sleep
    syscall
    bgez  $2,Sleep_ok
    nop
    # Error handling
    subu   $3,$0,$2
    sw     $3,errno
    addiu  $2,$0,-1
    j      $31
    nop
Sleep_ok:
    sw     $0,errno
    j      $31
    .end Sleep
    
/* SleepUntil : with error handling */
    .globl SleepUntil
    .ent	SleepUntil
SleepUntil:
    addiu $2,$0,SC_SleepUntil
    syscall
    bgez  $2,SleepUntil_ok
    nop
    # Error handling
    subu   $3,$0,$2
    sw     $3,errno
    addiu  $2,$0,-1
    j      $31
    nop
SleepUntil_ok:
    sw     $0,errno
    j      $31
    .end SleepUntil
    
/* GetCurrentTick : with error handling */
    .globl GetCurrentTick
    .ent	GetCurrentTick
GetCurrentTick:
    addiu $2,$0,SC_GetCurrentTick
    syscall
    bgez  $2,GetCurrentTick_ok
    nop
    # Error handling
    subu   $3,$0,$2
    sw     $3,errno
    addiu  $2,$0,-1
    j      $31
    nop
GetCurrentTick_ok:
    sw     $0,errno
    j      $31
    .end GetCurrentTick

/* -------------------------------------------------------------
 * Errno utility functions
 * -------------------------------------------------------------
 */

	.globl GetLastError
	.ent	GetLastError
GetLastError:
	lw    $2,errno		# Load errno into $v0
	j     $31		# Return
	.end GetLastError

	.globl ClearError
	.ent	ClearError
ClearError:
	sw    $0,errno		# errno = 0
	j     $31		# Return
	.end ClearError

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
	.globl  errno
errno:
	.word   0		/* errno initialized to 0 */
