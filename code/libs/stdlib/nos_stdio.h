#ifndef NOS_STDIO_H
#define NOS_STDIO_H

#include "nos_stdarg.h"
#include "nos_stddef.h"

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef unsigned long size_t;
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

/* ============================================================
 * Constants
 * ============================================================
 */
#define EOF (-1)

#define BUFSIZ 1024

#define _IOFBF 0  // Fully buffered
#define _IOLBF 1  // Line buffered
#define _IONBF 2  // Unbuffered

#define _F_READ    0x0001  // File open for reading
#define _F_WRITE   0x0002  // File open for writing
#define _F_EOF     0x0010  // EOF reached
#define _F_ERR     0x0020  // Error occurred
#define _F_MYBUF   0x0040  // Buffer was allocated by stdio
#define _F_LINEBUF 0x0080  // Line buffered
#define _F_NOBUF   0x0100  // Unbuffered

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

/* ============================================================
 * FILE structure
 * ============================================================
 */
typedef struct FILE {
    int fd;
    char *buffer;
    size_t buf_size;
    size_t buf_pos;
    size_t buf_end;
    int flags;
    int ungetc_buf;
} FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

/* ============================================================
 * Initialization (call before using stdio)
 * ============================================================
 */

/**
 * @brief Initialize stdio subsystem
 *
 * Must be called before any stdio function.
 * Sets up stdin, stdout, stderr with appropriate buffering.
 *
 * @return 0 on success, -1 on failure
 */
int _stdio_init(void);

/**
 * @brief Cleanup stdio subsystem
 *
 * Flushes all buffers and frees resources.
 * Called automatically by exit() if registered.
 */
void _stdio_cleanup(void);

/* ============================================================
 * Buffer control
 * ============================================================
 */

/**
 * @brief Set stream buffering mode
 *
 * @param stream Target stream
 * @param buf Buffer to use (NULL for auto-allocation or unbuffered)
 * @param mode Buffering mode (_IOFBF, _IOLBF, or _IONBF)
 * @param size Buffer size (ignored if mode is _IONBF)
 * @return 0 on success, non-zero on failure
 */
int setvbuf(FILE *stream, char *buf, int mode, size_t size);

/**
 * @brief Set stream buffering (simplified)
 *
 * @param stream Target stream
 * @param buf Buffer to use (NULL for unbuffered)
 */
void setbuf(FILE *stream, char *buf);

/**
 * @brief Flush stream buffer
 *
 * @param stream Stream to flush (NULL to flush all streams)
 * @return 0 on success, EOF on error
 */
int fflush(FILE *stream);

/* ============================================================
 * Error handling
 * ============================================================
 */

/**
 * @brief Clear error and EOF indicators
 * @param stream Target stream
 */
void clearerr(FILE *stream);

/**
 * @brief Test EOF indicator
 * @param stream Target stream
 * @return Non-zero if EOF is set
 */
int feof(FILE *stream);

/**
 * @brief Test error indicator
 * @param stream Target stream
 * @return Non-zero if error is set
 */
int ferror(FILE *stream);

/* ============================================================
 * Unformatted input/output functions
 * ============================================================
 */

/**
 * @brief Read a character from stream
 * @param stream Input stream
 * @return Character read, or EOF on error/end-of-file
 */
int fgetc(FILE *stream);

/**
 * @brief Read a character from stream
 * @param stream Input stream
 * @return Character read, or EOF on error/end-of-file
 */
int getc(FILE *stream);

/**
 * @brief Read a character from stdin
 * @return Character read, or EOF on error/end-of-file
 */
int getchar(void);

/**
 * @brief Read a line from stream
 *
 * Reads at most n-1 characters, stopping at newline or EOF.
 * The newline is included if read. String is null-terminated.
 *
 * @param s Destination buffer
 * @param n Buffer size
 * @param stream Input stream
 * @return s on success, NULL on error or EOF with no characters read
 */
char *fgets(char *s, int n, FILE *stream);

/**
 * @brief Push character back to stream
 *
 * @param c Character to push back (converted to unsigned char)
 * @param stream Target stream
 * @return c on success, EOF on failure
 */
int ungetc(int c, FILE *stream);

/**
 * @brief Write a character to stream
 * @param c Character to write
 * @param stream Output stream
 * @return Character written, or EOF on error
 */
int fputc(int c, FILE *stream);

/**
 * @brief Write a character to stream (may be macro)
 * @param c Character to write
 * @param stream Output stream
 * @return Character written, or EOF on error
 */
int putc(int c, FILE *stream);

/**
 * @brief Write a character to stdout
 * @param c Character to write
 * @return Character written, or EOF on error
 */
int putchar(int c);

/**
 * @brief Write a string to stream
 *
 * Does not write the null terminator.
 *
 * @param s String to write
 * @param stream Output stream
 * @return Non-negative on success, EOF on error
 */
int fputs(const char *s, FILE *stream);

/**
 * @brief Write a string to stdout with newline
 *
 * Writes the string followed by a newline character.
 *
 * @param s String to write
 * @return Non-negative on success, EOF on error
 */
int puts(const char *s);

/* ============================================================
 * Formatted input/output functions
 * ============================================================
 */
int scanf(const char *format, ...);
int fscanf(FILE *stream, const char *format, ...);
int sscanf(const char *s, const char *format, ...);

int vscanf(const char *format, va_list args);
int vfscanf(FILE *stream, const char *format, va_list args);
int vsscanf(const char *s, const char *format, va_list args);

int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int sprintf(char *s, const char *format, ...);
int snprintf(char *s, size_t n, const char *format, ...);

int vprintf(const char *format, va_list args);
int vfprintf(FILE *stream, const char *format, va_list args);
int vsprintf(char *s, const char *format, va_list args);
int vsnprintf(char *s, size_t n, const char *format, va_list args);

#endif
