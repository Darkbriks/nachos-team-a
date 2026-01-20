
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

/* GET file from server and save to local filesystem */
int clientGetFile(int connId, char *remoteFile, char *localFile, int maxSize,
                  int *receivedSize, time_t *startTime, time_t *endTime);

/* PUT local file to server */
int clientPutFile(int connId, char *localFile, char *remoteFile, int maxSize,
                  time_t *startTime, time_t *endTime);

/* Calculate and display throughput using real time */
void clientDisplayThroughput(int bytes, time_t startTime, time_t endTime);

#endif /* NOS_CLIENT_H */
