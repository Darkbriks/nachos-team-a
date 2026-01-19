
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



#include "syscall.h"
#include "nos_stdio.h"
#include "nos_errno.h"
#include "nos_string.h"

#define PORT 20

#define CHUNK_SIZE 20  /* Safe size for data chunks */
#define MAX_FILENAME 64
#define MAX_FILESIZE 4096

/* Simple file storage (simulated in memory for testing) */
char fileBuffer[MAX_FILESIZE];
int fileSize = 0;
char storedFilename[MAX_FILENAME];


/* Convert int to string (simple itoa) */
int intToStr(int num, char *buf);




/* Parse command from received data */
int parseCommand(char *data, char *cmd, char *arg1, int *arg2);


/* Handle GET request - send file to client */
int handleGet(int connId, char *filename);



/* Handle PUT request - receive file from client */
int handlePut(int connId, char *filename, int size);


