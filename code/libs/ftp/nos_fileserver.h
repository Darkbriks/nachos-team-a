
/**
 * File Transfer Server for NachOS
 *
 * This server listens for incoming connections and handles
 * file transfer requests (GET/PUT operations).
 *
 * Protocol:
 *   GET <filename>       - Client requests a file
 *   PUT <filename> <size> - Client sends a file
 *   OK <size>            - Server confirms GET (sends file size)
 *   OK                   - Server confirms PUT receipt
 *   ERR <code>           - Error response
 *   DATA <bytes>         - File data chunk
 *   EOF                  - End of file transfer
 */

#ifndef NOS_FILESERVER_H
#define NOS_FILESERVER_H

#include "syscall.h"
#include "nos_stdio.h"
#include "nos_errno.h"
#include "nos_string.h"

#define SERVER_PORT 20

#define SERVER_CHUNK_SIZE 20  /* Safe size for data chunks */
#define SERVER_MAX_FILENAME 64
#define SERVER_MAX_FILESIZE 4096

/* Parse command from received data */
int serverParseCommand(char *data, char *cmd, char *arg1, int *arg2);

/* Handle GET request - send file to client */
int serverHandleGet(int connId, char *filename);

/* Handle PUT request - receive file from client */
int serverHandlePut(int connId, char *filename, int size);

#endif /* NOS_FILESERVER_H */
