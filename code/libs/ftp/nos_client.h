
/**
 * File Transfer Client for NachOS
 *
 * This client connects to a file server and performs
 * file transfer operations (GET/PUT).
 *
 * Usage: Performs a GET operation to download a test file,
 *        measures transfer time, and calculates throughput.
 */

#ifndef NOS_CLIENT_H
#define NOS_CLIENT_H

#include "syscall.h"
#include "nos_stdio.h"
#include "nos_errno.h"
#include "nos_string.h"

#define CLIENT_SERVER_ADDR 0
#define CLIENT_SERVER_PORT 20  /* Must match SERVER_PORT in nos_fileserver.h */
#define CLIENT_CHUNK_SIZE 20
#define CLIENT_MAX_FILESIZE 4096

/* Parse OK response to extract size */
int clientParseOkResponse(char *response, int *size);

/* GET file from server */
int clientGetFile(int connId, char *filename, char *buffer, int bufSize,
                  int *receivedSize, long long *startTime, long long *endTime);

/* PUT file to server */
int clientPutFile(int connId, char *filename, char *data, int size,
                  long long *startTime, long long *endTime);

/* Calculate and display throughput */
void clientDisplayThroughput(int bytes, long long startTime, long long endTime);

#endif /* NOS_CLIENT_H */
