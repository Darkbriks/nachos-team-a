#ifndef NOS_COMMON_H
#define NOS_COMMON_H

#define CHUNK_SIZE 20  /* Safe size for data chunks */
#define MAX_FILESIZE 4096
#define MAX_FILENAME 64
#define PORT 20  /* Must match SERVER_PORT in nos_fileserver.h */
#define SERVER_ADDR 0

int connectRequest(int connId, char* cmd_name, char* cmd);
int networkPUT(int connId, char* filename);
int networkGET(int connId, char* filename, int size);
int sendFile(int fd, int connId);

#endif // NOS_COMMON_H
