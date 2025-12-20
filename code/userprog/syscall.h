/* syscalls.h
 * 	Nachos system call interface.  These are Nachos kernel operations
 * 	that can be invoked from user programs, by trapping to the kernel
 *	via the "syscall" instruction.
 *
 *	This file is included by user programs and by the Nachos kernel.
 *
 * Copyright (c) 1992-1993 The Regents of the University of California.
 * All rights reserved.  See copyright.h for copyright notice and limitation
 * of liability and disclaimer of warranty provisions.
 */

#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "copyright.h"

/* system call codes -- used by the stubs to tell the kernel which system call
 * is being asked for
 */
#define SC_Halt 0
#define SC_Exit 1

#define SC_Exec 2
#define SC_Join 3

#define SC_Create 4
#define SC_Open 5
#define SC_Read 6
#define SC_Write 7
#define SC_Close 8

#define SC_Fork 9
#define SC_Yield 10

#define SC_PutChar 11
#define SC_PutString 12
#define SC_GetChar 13
#define SC_GetString 14
#define SC_PutInt 15
#define SC_GetInt 16

#define SC_CreateThread 17
#define SC_ExitThread 18
#define SC_JoinThread 19

#define SC_SemInit 20
#define SC_SemP 21
#define SC_SemV 22
#define SC_SemDestroy 23

/* Error codes - returned as negative values by syscalls, stored as positive in errno */
#define E_SUCCESS       0   /* No error */
#define E_INVAL         1   /* Invalid argument */
#define E_FAULT         2   /* Bad address / memory access error */
#define E_OVERFLOW      3   /* Arithmetic overflow */
#define E_IO            4   /* I/O error */
#define E_FORMAT        5   /* Invalid format */
#define E_EOF           6   /* End of file */
#define E_NOMEM         7   /* Out of memory */
#define E_RANGE         8   /* Result out of range */
#define E_NOSPC         9   /* No such process */

#ifdef IN_USER_MODE

// LB: This part is read only on compiling the test/*.c files.
// It is *not* read on compiling test/start.S

/* The system call interface.  These are the operations the Nachos
 * kernel needs to support, to be able to run user programs.
 *
 * Each of these is invoked by a user program by simply calling the
 * procedure; an assembly language stub stuffs the system call code
 * into a register, and traps to the kernel.  The kernel procedures
 * are then invoked in the Nachos kernel, after appropriate error checking,
 * from the system call entry point in exception.cc.
 */


/* -------------------------------------------------------------
 * ERROR HANDLING
 * -------------------------------------------------------------
 */

/**
 * @brief Global error variable (defined in start.S)
 * Set to 0 on successful syscall, or error code (E_xxx) on failure
 */
extern int errno;

/**
 * @brief Get the last error code
 * @return Current value of errno
 */
int GetLastError(void);

/**
 * @brief Clear the error code (set errno to 0)
 */
void ClearError(void);


/* -------------------------------------------------------------
 * SYSTEM CONTROL OPERATIONS: Halt
 * -------------------------------------------------------------
 */

/* Stop Nachos, and print out performance stats */
void Halt() __attribute__((noreturn));


/* -------------------------------------------------------------
 * ADDRESS SPACE CONTROL OPERATIONS: Exit, Exec, Join
 * -------------------------------------------------------------
 */

/* This user program is done (status = 0 means exited normally). */
void Exit(int status) __attribute__((noreturn));

/* A unique identifier for an executing user program (address space) */
typedef int SpaceId;

/* Run the executable, stored in the Nachos file "name", and return the
 * address space identifier
 */
SpaceId Exec(char *name);

/* Only return once the the user program "id" has finished.
 * Return the exit status.
 */
int Join(SpaceId id);


/* -------------------------------------------------------------
 * FILE SYSTEM OPERATIONS : Create, Open, Read, Write, Close
 *
 * These functions are patterned after UNIX -- files represent
 * both files *and* hardware I/O devices.
 *
 * If this assignment is done before doing the file system assignment,
 * note that the Nachos file system has a stub implementation, which
 * will work for the purposes of testing out these routines.
 * -------------------------------------------------------------
 */

/* A unique identifier for an open Nachos file. */
typedef int OpenFileId;

/* when an address space starts up, it has two open files, representing
 * keyboard input and display output (in UNIX terms, stdin and stdout).
 * Read and Write can be used directly on these, without first opening
 * the console device.
 */

#define ConsoleInput 0
#define ConsoleOutput 1

/* Create a Nachos file, with "name" */
void Create(char *name);

/* Open the Nachos file "name", and return an "OpenFileId" that can
 * be used to read and write to the file.
 */
OpenFileId Open(char *name);

/* Write "size" bytes from "buffer" to the open file. */
void Write(char *buffer, int size, OpenFileId id);

/* Read "size" bytes from the open file into "buffer".
 * Return the number of bytes actually read -- if the open file isn't
 * long enough, or if it is an I/O device, and there aren't enough
 * characters to read, return whatever is available (for I/O devices,
 * you should always wait until you can return at least one character).
 */
int Read(char *buffer, int size, OpenFileId id);

/* Close the file, we're done reading and writing to it. */
void Close(OpenFileId id);


/* -------------------------------------------------------------
 * USER-LEVEL THREAD OPERATIONS : Fork, Yield
 * -------------------------------------------------------------
 */

/* Fork a thread to run a procedure ("func") in the *same* address space
 * as the current thread.
 */
void Fork(void (*func)());

/* Yield the CPU to another runnable thread, whether in this address space
 * or not.
 */
void Yield();


/* -------------------------------------------------------------
 * CONSOLE I/O OPERATIONS : PutChar, PutString, PutInt, GetChar, GetString, GetInt
 * -------------------------------------------------------------
 */

/**
 * @brief Write a char in the console 
 *
 * @param c  The character to print on the console
 */
void PutChar(char c);

/**
 * @brief Write a String on the console
 * @note '\0' is implicitly added at the end of the string,
 *		so you don't need to count it in n
 *
 * @code int n = PutString("Hello World\n", 12);
 * @param s  The string to print on the console
 * @param n Maximum number of bytes to write (stops at '\0' or after n bytes)
 * @return The number of bytes effectively written, or -1 on error (check errno)
 * @warning if n > MAX_PUT_STRING then we only display the MAX_PUT_STRING first characters
 */
int PutString(char *s, int n);

/**
 * @brief Write an integer on a console
 *
 * @param n The integer to write
 */
void PutInt(int n);

/**
 * @brief Read a character from console
 *
 * @return The character read (or EOF)
 */
char GetChar();

/**
 * @brief Read a string from console
 *
 * @param s Buffer to store string (must be allocated by caller)
 * @param n Maximum number of characters to read (including '\0')
 * @return Number of bytes read (excluding '\0'), or -1 on error (check errno)
 *
 * @note Stops at newline ('\n') or after n-1 characters
 * @note Always null-terminates the string
 * @warning Buffer must be at least n bytes large
 */
int GetString(char *s, int n);

/**
 * @brief Read an integer from a console
 *                                                                                                                                                                              
 * @param n A pointer on an integer. This is where the result will be put
 * @warning If the string provided can't be cast in integer we exit the programm
 */
int GetInt(int *n);

/**
 * @brief Create a thread at the user level
 *
 * @param f Function to execute
 * @param arg Argument for the function
 * @param arg Argument for the function
 * @return  the tid of the thread newly created
 */
int CreateThread(void f(void *arg), void *arg);

/**
 * @brief The caller will be kill if this function works
 */
void ExitThread();

/**
 * @brief The caller wait for the thread finish
 *
 * @param TID The thread to wait
 */
void JoinThread(int TID);

typedef int sem_t;
/**
 * @brief Init a Semaphore with original value of value 
 *
 * @param sem  The adress of a semaphore sem_t not initialized before
 * @param value  The original number of thread can P this semaphore 
 * @return 0 if everything is fine, or -1 on error (check errno)
 */
int  SemInit(sem_t *sem, int value);

/**
 * @brief Semaphore must be init before call this, try to take one of the sem token, wait while value <= 0 
 *
 * @param sem  A pointer on a semaphore previously init
 * @return 0 if everything is fine, or -1 on error (check errno)
 */
int  SemP(sem_t *sem);

/**
 * @brief Semaphore must be init before call this, post one of the sem token if the caller has one ;
 *
 * @param sem  A pointer on a semaphore previously init
 * @return 0 if everything is fine, or -1 on error (check errno)
 */
int  SemV(sem_t *sem);

/**
 * @brief destroy a sem previously init 
 *
 * @param sem  A pointer on a semaphore previously init
 * @return 0 if everything is fine, or -1 on error (check errno)
 */
int  SemDestroy(sem_t *sem);


#endif // IN_USER_MODE

#endif /* SYSCALL_H */
