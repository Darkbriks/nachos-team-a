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

#define MAX_PUT_STRING 8192
#define MAX_STRING_SIZE 256
#define MAX_PATH_SIZE 1024

/* system call codes -- used by the stubs to tell the kernel which system call
 * is being asked for
 */

/* --- System Control --- */
#define SC_Halt 0

/* --- Console I/O --- */
#define SC_PutChar   1
#define SC_PutString 2
#define SC_GetChar   3
#define SC_GetString 4
#define SC_PutInt    5
#define SC_GetInt    6

/* --- Time --- */
#define SC_Sleep          7
#define SC_SleepUntil     8
#define SC_GetCurrentTick 9

/* --- Semaphores --- */
/* DEPRECATED, use userspace threads
 * synchronization primitives instead */
#define SC_SemInit             10
#define SC_SemWait             11
#define SC_SemPost             12
#define SC_SemDestroy          13
#define SC_SetMaxSemForProcess 14

/* --- Futexes --- */
#define SC_futex_wait     15
#define SC_futex_wake     16
#define SC_atomic_cmpxchg 17
#define SC_atomic_store   18
#define SC_atomic_load    19

/* --- Memory Management --- */
#define SC_Sbrk   20
#define SC_mmap   21
#define SC_munmap 22

/* --- Threads --- */
#define SC_thread_create 23
#define SC_thread_exit   24
#define SC_thread_self   25
#define SC_thread_yield  26

/* --- Process Management --- */
#define SC_ForkExec 27
#define SC_ForkJoin 28
#define SC_ForkSelf 29
#define SC_Exit     30

/* Network */
#define SC_connect  31
#define SC_listen   32
#define SC_accept   33
#define SC_sendto   34
#define SC_recvfrom 35
#define SC_close    36

/* --- File System --- */
#define SC_Create 37
#define SC_Open   38
#define SC_Read   39
#define SC_Write  40
#define SC_Close  41

#define SC_time 42
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

/* Create a Nachos file, with "name" 
 * return true on succes and false on error
 */
int Create(char *name, int size);

/* Open the Nachos file "name", and return an "OpenFileId" that can
 * be used to read and write to the file.
 */
OpenFileId Open(char *name, int size);

/* Write "size" bytes from "buffer" to the open file. */
int Write(char *buffer, int size, OpenFileId id);

/* Read "size" bytes from the open file into "buffer".
 * Return the number of bytes actually read -- if the open file isn't
 * long enough, or if it is an I/O device, and there aren't enough
 * characters to read, return whatever is available (for I/O devices,
 * you should always wait until you can return at least one character).
 */
int Read(char *buffer, int size, OpenFileId id);

/* Close the file, we're done reading and writing to it. */
int Close(OpenFileId id);
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
 * @param sem_id Semaphore ID
 * @return 0 on success, negative error code on failure
 *
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/sync/SemWait.html">Full documentation</a>
 */
int SemWait(int sem_id);

/**
 * @brief Signal (V) operation on semaphore
 * @param sem_id Semaphore ID
 * @return 0 on success, negative error code on failure
 *
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/sync/SemPost.html">Full documentation</a>
 */
int SemPost(int sem_id);

/**
 * @brief Destroy a semaphore
 * @param sem_id Semaphore ID
 * @return 0 on success, negative error code on failure
 *
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/sync/SemDestroy.html">Full documentation</a>
 */
int SemDestroy(int sem_id);

/**
 * @brief Set maximum number of semaphores for the current process
 * @param maxSemaphores Maximum number of semaphores
 * @return Previous maximum number of semaphores
 *
 * <a href="https://darkbriks.github.io/nachos-team-a/syscalls/sync/SetMaxSemForProcess.html">Full documentation</a>
 */
int SetMaxSemForProcess(unsigned int maxSemaphores);

/* -------------------------------------------------------------
 * FUTEX OPERATIONS
 * -------------------------------------------------------------
 */

/**
 * @brief Wait on a futex
 * @param uaddr Address of the futex
 * @param expected Expected value at the futex address
 * @return 0 on success, negative error code on failure
 */
int futex_wait(int* uaddr, int expected);

/**
 * @brief Wake up threads waiting on a futex
 * @param uaddr Address of the futex
 * @param num_wake Number of threads to wake up
 * @return Number of threads actually woken up, negative error code on failure
 */
int futex_wake(int* uaddr, int num_wake);

/**
 * @brief Atomic compare and exchange operation
 * @param uaddr Address of the integer to operate on
 * @param expected Expected value
 * @param desired Desired value to set if comparison succeeds
 * @return The original value at the address
 */
int atomic_cmpxchg(int* uaddr, int expected, int desired);

/**
 * @brief Atomic store operation
 * @param uaddr Address of the integer to store to
 * @param value Value to store
 */
void atomic_store(int* uaddr, int value);

/**
 * @brief Atomic load operation
 * @param uaddr Address of the integer to load from
 * @return The loaded value
 */
int atomic_load(int* uaddr);

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

/**
 * @brief Create a new memory mapping in the process's address space
 * @param addr Desired starting address for the mapping (can be nullptr)
 * @param length Length of the mapping in bytes (must be multiple of page size)
 * @return Starting address of the mapping on success, negative error code on failure
 */
int mmap(void* addr, int length);

/**
 * @brief Remove a memory mapping from the process's address space
 * @param addr Starting address of the mapping
 * @return 0 on success, negative error code on failure
 */
int munmap(void* addr);

/* -------------------------------------------------------------
 * REWORK OF THREADS
 * -------------------------------------------------------------
 */

/**
 * @brief Create a new thread in the current process
 *
 * @param args Pointer to a thread_args structure containing thread parameters
 * @return 0 on success, negative error code on failure
 */
int thread_create(void* args);

/**
 * @brief Exit the current thread
 */
void thread_exit();

/**
 * @brief Get the TID of the current thread
 * @return The TID of the current thread
 */
int thread_self();

/**
 * @brief Yield the CPU to another thread
 */
void thread_yield();

/* -------------------------------------------------------------
 * PROCESS MANAGEMENT
 * -------------------------------------------------------------
 */

typedef int posix_process_t;

/**
 * @brief Fork and execute a new process
 * @param name Name of the executable file
 * @param size Size of the name string
 * @return Process ID on success, negative error code on failure
 */
posix_process_t ForkExec(char *name, int size);

/**
 * @brief  Wait for the process finish
 *
 * @param PID the PID of the process to wait
 * @param adrr_result A previously allocated adress where the exitcode will be put
 * @return 0 on sucess and -1 on error
 */
int ForkJoin(posix_process_t PID, int *adrr_result);

/**
 * @brief Get the PID of the current process
 * @return The PID of the current process
 */
int ForkSelf();

/**
 * @brief Terminate the current process with a status code
 * @param status Exit status code
 */
void Exit(int status) __attribute__((noreturn));

/* -------------------------------------------------------------
 * NETWORK
 * -------------------------------------------------------------
 */

/**
 * @brief Establish a connection to a remote server
 *
 * @param remoteAddr Network address of the remote machine
 * @param remotePort Port number on the remote machine
 * @param localPort  Local port to use (0 = auto-allocate)
 * @return Connection ID (>= 0) on success, negative error code on failure
 */
int connect(int remoteAddr, int remotePort, int localPort);

/**
 * @brief Start listening for incoming connections on a port
 *
 * @param port Port number to listen on (1-65535)
 * @return Listener ID (>= 0) on success, negative error code on failure
 */
int listen(int port);

/**
 * @brief Accept an incoming connection on a listening port
 *
 * @param listenerId ID returned by listen()
 * @param timeoutMs  Timeout in milliseconds (-1 = infinite, 0 = non-blocking)
 * @return Connection ID (>= 0) on success, negative error code on failure
 */
int accept(int listenerId, int timeoutMs);

/**
 * @brief Send data on an established connection
 *
 * @param connId Connection ID returned by connect() or accept()
 * @param data   Pointer to data buffer
 * @param size   Number of bytes to send
 * @return Number of bytes sent on success, negative error code on failure
 */
int sendto(int connId, char* data, int size);

/**
 * @brief Receive data from an established connection
 *
 * @param connId Connection ID returned by connect() or accept()
 * @param buffer Pointer to receive buffer
 * @param size   Maximum bytes to receive
 * @return Number of bytes received on success, 0 on EOF (peer closed),
 *         negative error code on failure
 */
int recvfrom(int connId, char* buffer, int size);

/**
 * @brief Close a network connection or listener
 *
 * @param id Connection ID or Listener ID
 * @return 0 on success, negative error code on failure
 *
 * For connections: initiates graceful close (FIN handshake)
 * For listeners: stops accepting new connections
 */
int close(int id);

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


/*
 * Retrieves the current calendar time (wall clock time) in seconds since the Unix epoch 
 * (January 1, 1970, 00:00:00 UTC)
 */
typedef long long time_t;

int time(time_t *loc);

#endif // IN_USER_MODE

#endif /* SYSCALLS_H */
