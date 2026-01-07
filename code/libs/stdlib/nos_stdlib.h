#ifndef STD_LIB_C
#define STD_LIB_C

#include "syscall.h"
#include "types.h"

/* =============================================================
 * I/O Functions
 * =============================================================
 */

/**
 * @brief Simple printf function
 * For now, only supports strings (no format specifiers)
 * @param buf The format string
 */
void printf_simple(char *buf);

/**
 * @brief Print the error contained in errno.
 * As a side effect, clears this value
 * @param msg  The string displayed before errno. Can be NULL to display nothing else than errno
 */
void print_error(char *msg);

/**
 * @brief Get a value from the console
 *
 * @param format The format we want to read like %s, %d, %c
 * @param result a pointer on a type accorded to the one ask in format
 * @return 0 if everything is ok -1 else
 */
int scanf_simple(char *format, void *result);

/* ============================================================
 * Conversion functions
 * ============================================================
 */

/**
 * @brief Convert an integer to a string in a given base
 * Only base 10 can be signed
 * @param value The integer value to convert
 * @param str The output string buffer
 * @param base The base for conversion (e.g., 10 for decimal, 16 for hexadecimal)
 * @return The output string buffer
 */
char* itoa(int value, char* str, int base);

/**
 * @brief Convert a string to an integer in a given base
 * @param str The input string
 * @param base The base for conversion (e.g., 10 for decimal, 16 for hexadecimal)
 * @return The converted integer value
 */
int atoi(const char* str, int base);

/* ============================================================
 * Old functions
 * ============================================================
 */

//void my_scanf(char *format, ...);

/*#ifndef BUFFER_SIZE
#define BUFFER_SIZE 10
#endif*/

/*typedef struct IOBUF_FILE
{
    int file_descriptor;
    char buffer[BUFFER_SIZE];
    char* start_buff;
    char* end_buff;
    int empty;
    char mode;
} IOBUF_FILE;*/

/* ----------------------------------------------------------*/
/* Interface utilisateur bibliothèque d'entrées/sorties      */
/* ----------------------------------------------------------*/
/*IOBUF_FILE* iobuf_open(char* nom, char mode);
int iobuf_close(IOBUF_FILE* f);
int iobuf_read(void* p, unsigned int taille, unsigned int nbelem, IOBUF_FILE * f);
int iobuf_write(void* p, unsigned int taille, unsigned int nbelem, IOBUF_FILE * f);

int iobuf_fprintf(IOBUF_FILE* fp, char* format, ...);
int iobuf_fscanf(IOBUF_FILE* fp, char* format, ...);

ssize_t iobuf_flush(IOBUF_FILE* f);
ssize_t iobuf_fill(IOBUF_FILE* f);

int close(int);
int open(char *, int);
int write(int fd, char *buf, size_t size);
ssize_t read(int fd, char *buf, size_t size);*/

/*void * malloc(unsigned int);
int free(void *);*/

/*#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define O_CREAT 3*/

#endif //STD_LIB_C
