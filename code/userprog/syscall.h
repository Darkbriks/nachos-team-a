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

/* --- System Control --- */
#define SC_Halt 0
#define SC_Exit 1

/* --- Process Management --- */
#define SC_Exec 2
#define SC_Join 3

/* --- File System --- */
#define SC_Create 4
#define SC_Open 5
#define SC_Read 6
#define SC_Write 7
#define SC_Close 8

#define SC_Fork 9
#define SC_Yield 10

/* --- Console I/O --- */
#define SC_PutChar 11
#define SC_PutString 12
#define SC_GetChar 13
#define SC_GetString 14
#define SC_PutInt 15
#define SC_GetInt 16

/* --- Pthreads --- */
#define SC_PthreadCreate 17
#define SC_PthreadExit 18
#define SC_PthreadJoin 19
#define SC_PthreadDetach 20
#define SC_PthreadSelf 21

/* --- Pthread attributes --- */
#define SC_Pthread_attr_init 22
#define SC_Pthread_attr_destroy 23
#define SC_Pthread_attr_setdetachstate 24
#define SC_Pthread_attr_getdetachstate 25

/* --- Time --- */
#define SC_Sleep 26
#define SC_SleepUntil 27
#define SC_GetCurrentTick 28

/* --- Semaphores --- */
#define SC_SemInit 29
#define SC_SemWait 30
#define SC_SemPost 31
#define SC_SemDestroy 32
#define SC_SetMaxSemForProcess 33

/* --- Process Management --- */
#define SC_ForkExec 34
#define SC_ForkJoin 35

/* --- Memory Management --- */
#define SC_Sbrk 36

/* Rework of threads */
#define SC_SetTLS 50
#define SC_GetTLS 51
#define SC_GetTID 52

/* ============================================================
 * ERROR CODES
 * Returned as negative values by syscalls, stored as positive in errno
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/errno.html">Documentation</a>
 * ============================================================
 */
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
#define E_FTABLE        10  /* Allocation table full (file table, semaphore table, etc.) */
#define E_NOENT         11  /* No such file or directory or table entry */
#define E_NOCPC         12   /* Not a child process */

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


/* ============================================================
 * ERROR HANDLING
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/errors.html">Documentation</a>
 * ============================================================
 */

/**
 * @brief Global error variable (defined in start.S)
 * Set to 0 on successful syscall, or error code (E_xxx) on failure
 */
extern int errno;

/**
 * @brief Get the last error code
 * @return Current value of errno
 *
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/errors.html#GetLastError">Full documentation</a>
 */
int GetLastError(void);

/**
 * @brief Clear the error code (set errno to 0)
 *
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/errors.html#ClearError">Full documentation</a>
 */
void ClearError(void);


/* -------------------------------------------------------------
 * SYSTEM CONTROL
 * -------------------------------------------------------------
 */

/**
 * @brief Stop Nachos, and print out performance stats
 */
void Halt() __attribute__((noreturn));


/* -------------------------------------------------------------
 * PROCESS MANAGEMENT
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
 * FILE SYSTEM OPERATIONS
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
 * USER-LEVEL THREAD OPERATIONS
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
 * CONSOLE I/O
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/console/Console.html">Full documentation</a>
 * -------------------------------------------------------------
 */

/**
 * @brief Output a character to the console
 * @param c Character to output
 *
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/console/PutChar.html">Full documentation</a>
 */
void PutChar(char c);

/**
 * @brief Output a string to the console
 * @param s String to output
 * @param n Maximum number of characters to output
 * @return Number of characters actually output
 *
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/console/PutString.html">Full documentation</a>
 */
int PutString(char *s, int n);

/**
 * @brief Output an integer to the console
 * @param n Integer to output
 *
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/console/PutInt.html">Full documentation</a>
 */
void PutInt(int n);

/**
 * @brief Input a character from the console
 * @return The character read
 *
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/console/GetChar.html">Full documentation</a>
 */
char GetChar();

/**
 * @brief Input a string from the console
 * @param s Buffer to store the string
 * @param n Maximum number of characters to read
 * @return Number of characters actually read
 *
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/console/GetString.html">Full documentation</a>
 */
int GetString(char *s, int n);

/**
 * @brief Input an integer from the console
 * @param n Pointer to store the read integer
 * @return 0 on success, negative error code on failure
 *
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/console/GetInt.html">Full documentation</a>
 */
int GetInt(int *n);

/* -------------------------------------------------------------
 * POSIX THREAD OPERATIONS
 * -------------------------------------------------------------
 */

typedef unsigned int posix_thread_t;
typedef int posix_process_t;
typedef struct {} pthread_attr_t;

/**
 * @brief Create a new thread
 *
 * @param thread A pointer to store the created thread id
 * @param attr Thread attributes (can be nullptr for default attributes)
 * @param start_routine The function to be executed by the thread
 * @param arg The argument to be passed to the function
 * @return 0 on success, -1 on error (check errno)
 */
int PthreadCreate(posix_thread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);

/**
 * @brief Destruct the caller thread
 *
 * @param retval The return value of the thread
 * @return This function does not return
 */
void PthreadExit(void *retval);

/**
 * @brief Caller will wait other thread to finish
 *
 * @param thread The thread to wait
 * @param retval A pointer to store the return value of the joined thread (can be nullptr)
 * @return 0 on success, -1 on error (check errno)
 */
int PthreadJoin(posix_thread_t thread, void **retval);

/**
 * @brief Detach a thread
 *
 * @param thread The thread to detach
 * @return 0 on success, -1 on error (check errno)
 */
int PthreadDetach(posix_thread_t thread);

/**
 * @brief Get the TID of the current thread
 *
 * @return The TID on success ( >= 0 ), -1 on error (check errno)
 */
int PthreadSelf();

/* -------------------------------------------------------------
 * POSIX THREAD ATTRIBUTES OPERATIONS
 * -------------------------------------------------------------
 */
/**
 * @brief Initialize thread attributes object.
 *
 * @param attr Pointer to the thread attributes object to initialize.
 * @return 0 on success, -1 on error (check errno)
 */
int Pthread_attr_init(pthread_attr_t *attr);

/**
 * @brief Destroy thread attributes object.
 *
 * @param attr Pointer to the thread attributes object to destroy.
 * @return 0 on success, -1 on error (check errno)
 */
int Pthread_attr_destroy(pthread_attr_t *attr);

/**
 * @brief Set the detach state attribute in the thread attributes object.
 *
 * @param attr Pointer to the thread attributes object.
 * @param detachstate Desired detach state (JOINABLE or DETACHED).
 * @return 0 on success, -1 on error (check errno)
 */
int Pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);

/**
 * @brief Get the detach state attribute from the thread attributes object.
 *
 * @param attr Pointer to the thread attributes object.
 * @param detachstate Pointer to store the retrieved detach state.
 * @return 0 on success, -1 on error (check errno)
 */
int Pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);

/* -------------------------------------------------------------
 * TIME OPERATIONS
 * -------------------------------------------------------------
 */

/**
 * @brief Put the calling thread to sleep for a number of ticks
 *
 * @param sleep_time Number of ticks to sleep
 * @return 0 on success, -1 on error (check errno)
 */
int Sleep(int sleep_time);

/**
 * @brief Put the calling thread to sleep until a specific time
 *
 * @param wake_time The time (in ticks) to wake up
 * @return 0 on success, -1 on error (check errno)
 */
int SleepUntil(long long wake_time);

/**
 * @brief Get the current tick count
 *
 * @param tick A pointer to store the current tick count
 * @return 0 on success, -1 on error (check errno)
 */
int GetCurrentTick(long long *tick);

/* -------------------------------------------------------------
 * SEMAPHORE OPERATIONS
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/sync/Sync.html">Full documentation</a>
 * -------------------------------------------------------------
 */

/**
 * @brief Initialize a semaphore
 * @param value Initial value of the semaphore
 * @return Semaphore ID on success, negative error code on failure
 *
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/sync/SemInit.html">Full documentation</a>
 */
int SemInit(int value);

/**
 * @brief Wait (P) operation on semaphore
 * @param semId Semaphore ID
 * @return 0 on success, negative error code on failure
 *
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/sync/SemWait.html">Full documentation</a>
 */
int SemWait(int sem_id);

/**
 * @brief Signal (V) operation on semaphore
 * @param semId Semaphore ID
 * @return 0 on success, negative error code on failure
 *
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/sync/SemPost.html">Full documentation</a>
 */
int SemPost(int sem_id);

/**
 * @brief Destroy a semaphore
 * @param semId Semaphore ID
 * @return 0 on success, negative error code on failure
 *
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/sync/SemDestroy.html">Full documentation</a>
 */
int SemDestroy(int sem_id);

/**
 * @brief Set maximum number of semaphores for the current process
 * @param max Maximum number of semaphores
 * @return Previous maximum number of semaphores
 *
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/sync/SetMaxSemForProcess.html">Full documentation</a>
 */
int SetMaxSemForProcess(unsigned int maxSemaphores);

/* -------------------------------------------------------------
 * PROCESS MANAGEMENT
 * -------------------------------------------------------------
 */

/**
 * @brief Fork and execute a new process
 * @param name Name of the executable file
 * @return Process ID on success, negative error code on failure
 */
posix_process_t ForkExec(char *name);

/**
 * @brief  Wait for the process finish
 *
 * @param PID the PID of the process to wait
 * @param addr_result A previously allocated adress where the exitcode will be put
 * @return 0 on sucess and -1 on error
 */
int ForkJoin(posix_process_t PID, int *adrr_result);

/* -------------------------------------------------------------
 * MEMORY MANAGEMENT
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/memory/Memory.html">Full documentation</a>
 * -------------------------------------------------------------
 */

/**
 * @brief Change the size of the data segment (heap)
 * @param n Number of pages to increase (can be 0 to query current brk)
 * @return Pointer to the start of newly allocated memory, or -1 on error
 *
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/memory/Sbrk.html">Full documentation</a>
 */
int Sbrk(int n);

/* -------------------------------------------------------------
 * REWORK OF THREADS
 * -------------------------------------------------------------
 */

/**
 * @brief Set the Thread Local Storage base address for the current thread
 * The TLS block must be allocated in user space before calling.
 * @param addr Base address of the TLS area
 * @return 0 on success, negative error code on failure
 */
int SetTLS(void* addr);

/**
 * @brief Get the Thread Local Storage base address for the current thread
 * @return Base address of the TLS area, or 0 on error
 */
void* GetTLS();

/**
 * @brief Get the TID of the current thread
 *
 * @return The TID on success ( >= 0 ), -1 on error (check errno)
 */
int GetTID();

#endif // IN_USER_MODE

#endif /* SYSCALLS_H */
