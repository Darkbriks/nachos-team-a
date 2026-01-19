
/**
 * File Transfer Client for NachOS
 *
 * This client connects to a file server and performs
 * file transfer operations (GET/PUT).
 *
 * Usage: Performs a GET operation to download a test file,
 *        measures transfer time, and calculates throughput.
 */

#include "syscall.h"
#include "nos_stdio.h"
#include "nos_errno.h"
#include "nos_string.h"

#define SERVER_ADDR 0
#define SERVER_PORT 60
#define CHUNK_SIZE 20
#define MAX_FILESIZE 4096

char fileBuffer[MAX_FILESIZE];

/* Convert int to string (simple itoa) */
int intToStr(int num, char *buf);


/* Parse OK response to extract size */
int parseOkResponse(char *response, int *size);


/* GET file from server */
int getFile(int connId, char *filename, int *receivedSize,
        long long *startTime, long long *endTime);


/* PUT file to server */
int putFile(int connId, char *filename, char *data, int size,
                   long long *startTime, long long *endTime);


/* Calculate and display throughput */
void displayThroughput(int bytes, long long startTime, long long endTime);

