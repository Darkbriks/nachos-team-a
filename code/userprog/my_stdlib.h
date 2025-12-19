#ifndef MY_STD_LIB_C
#define MY_STD_LIB_C

#include "syscall.h"

#define NULL 0
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef unsigned char uint8_t;
typedef unsigned int size_t;
typedef int ssize_t;

void my_printf(char *buf);
void my_scanf(char *format, ...);

unsigned int my_strlen(char *str);

/**
 * @brief Affiche l'erreur contenue dans errno. Par effet de bord, détruit cette valeur
 *
 * @param msg  La chaine de caractére affichée avant errno. peut être  NULL pour ne rien afficher d'autre que errno
 */
void print_error(const char *msg); 
void my_strcpy(char *dest, char * src);
void my_memcpy(void *dest, void * src, size_t size);

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 10
#endif


typedef struct IOBUF_FILE
{
    int file_descriptor;
    char buffer[BUFFER_SIZE];
    char* start_buff;
    char* end_buff;
    int empty;
    char mode;
} IOBUF_FILE;

/* ----------------------------------------------------------*/
/* Interface utilisateur bibliothèque d'entrées/sorties      */
/* ----------------------------------------------------------*/
IOBUF_FILE* iobuf_open(char* nom, char mode);
int iobuf_close(IOBUF_FILE* f);
int iobuf_read(void* p, unsigned int taille, unsigned int nbelem, IOBUF_FILE * f);
int iobuf_write(void* p, unsigned int taille, unsigned int nbelem, IOBUF_FILE * f);

int iobuf_fprintf(IOBUF_FILE* fp, char* format, ...);
int iobuf_fscanf(IOBUF_FILE* fp, char* format, ...);

ssize_t iobuf_flush(IOBUF_FILE* f);
ssize_t iobuf_fill(IOBUF_FILE* f);

char *itos(int value, int base);

int close(int);
int open(char *, int);
int write(int fd, char *buf, size_t size);
ssize_t read(int fd, char *buf, size_t size);

void * malloc(unsigned int);
int free(void *);

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define O_CREAT 3



#endif //MY_STD_LIB_C
