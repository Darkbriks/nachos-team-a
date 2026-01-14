#include "nos_stdio.h"
#include "nos_stdlib.h"
#include "nos_string.h"
#include "nos_errno.h"
#include "syscall.h"

/* ============================================================
 * Internal I/O abstraction layer
 * ============================================================
 */

/**
 * @brief Low-level read from file descriptor
 *
 * @param fd File descriptor
 * @param buf Destination buffer
 * @param count Maximum bytes to read
 * @return Number of bytes read, 0 on EOF, -1 on error
 */
static int _io_read(int fd, char *buf, int count) {
    if (count <= 0) { return 0; }

    if (fd == STDIN_FILENO) {
        #if 1
            int i;
            for (i = 0; i < count; i++) {
                char c = GetChar();
                if (c == (char)EOF) { return (i > 0) ? i : 0; }
                buf[i] = c;
                if (c == '\n') { return i + 1; }
            }
            return i;
        #else
            return Read(buf, count, fd);
        #endif
    } else {
        errno = E_IO;
        return -1;  // Not implemented for non-console
    }
}

/**
 * @brief Low-level write to file descriptor
 *
 * @param fd File descriptor
 * @param buf Source buffer
 * @param count Number of bytes to write
 * @return Number of bytes written, -1 on error
 */
static int _io_write(int fd, const char *buf, int count) {
    if (count <= 0) { return 0; }

    if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
        #if 1
            for (int i = 0; i < count; i++) { PutChar(buf[i]); } return count;
        #else
            Write((char*)buf, count, fd); return count;
        #endif
    } else {
        errno = E_IO;
        return -1;  // Not implemented for non-console
    }
}

/* ============================================================
 * Static storage for standard streams
 * ============================================================
 */
static FILE _stdin_file;
static FILE _stdout_file;
static FILE _stderr_file;

static char _stdin_buffer[BUFSIZ];
static char _stdout_buffer[BUFSIZ];

FILE *stdin  = NULL;
FILE *stdout = NULL;
FILE *stderr = NULL;

static int _stdio_initialized = 0;

/* ============================================================
 * Initialization and cleanup
 * ============================================================
 */

int _stdio_init(void) {
    if (_stdio_initialized) { return 0; }

    _stdin_file.fd = STDIN_FILENO;
    _stdin_file.buffer = _stdin_buffer;
    _stdin_file.buf_size = BUFSIZ;
    _stdin_file.buf_pos = 0;
    _stdin_file.buf_end = 0;
    _stdin_file.flags = _F_READ | _F_LINEBUF;
    _stdin_file.ungetc_buf = -1;
    stdin = &_stdin_file;

    _stdout_file.fd = STDOUT_FILENO;
    _stdout_file.buffer = _stdout_buffer;
    _stdout_file.buf_size = BUFSIZ;
    _stdout_file.buf_pos = 0;
    _stdout_file.buf_end = 0;
    _stdout_file.flags = _F_WRITE | _F_LINEBUF;
    _stdout_file.ungetc_buf = -1;
    stdout = &_stdout_file;

    _stderr_file.fd = STDERR_FILENO;
    _stderr_file.buffer = NULL;
    _stderr_file.buf_size = 0;
    _stderr_file.buf_pos = 0;
    _stderr_file.buf_end = 0;
    _stderr_file.flags = _F_WRITE | _F_NOBUF;
    _stderr_file.ungetc_buf = -1;
    stderr = &_stderr_file;

    atexit(_stdio_cleanup);

    _stdio_initialized = 1;
    return 0;
}

void _stdio_cleanup(void) {
    if (!_stdio_initialized) { return; }

    fflush(stdout);
    fflush(stderr);

    if (stdin && (stdin->flags & _F_MYBUF) && stdin->buffer) {
        free(stdin->buffer);
        stdin->buffer = NULL;
    }
    if (stdout && (stdout->flags & _F_MYBUF) && stdout->buffer) {
        free(stdout->buffer);
        stdout->buffer = NULL;
    }
    if (stderr && (stderr->flags & _F_MYBUF) && stderr->buffer) {
        free(stderr->buffer);
        stderr->buffer = NULL;
    }

    _stdio_initialized = 0;
}

/* ============================================================
 * Internal helper: ensure stdio is initialized
 * ============================================================
 */
static inline int _ensure_init(void) {
    if (!_stdio_initialized) { return _stdio_init(); }
    return 0;
}

/* ============================================================
 * Buffer control
 * ============================================================
 */

int setvbuf(FILE *stream, char *buf, int mode, size_t size) {
    if (stream == NULL) { return -1; }
    if (mode != _IOFBF && mode != _IOLBF && mode != _IONBF) { return -1; }

    if (stream->flags & _F_WRITE) { fflush(stream); }

    if ((stream->flags & _F_MYBUF) && stream->buffer) {
        free(stream->buffer);
        stream->flags &= ~_F_MYBUF;
    }

    stream->flags &= ~(_F_LINEBUF | _F_NOBUF);

    if (mode == _IONBF) {
        stream->buffer = NULL;
        stream->buf_size = 0;
        stream->flags |= _F_NOBUF;
    } else {
        if (buf != NULL) {
            stream->buffer = buf;
            stream->buf_size = size;
        } else {
            stream->buffer = (char*)malloc(size > 0 ? size : BUFSIZ);
            if (stream->buffer == NULL) { return -1; }
            stream->buf_size = size > 0 ? size : BUFSIZ;
            stream->flags |= _F_MYBUF;
        }

        if (mode == _IOLBF) { stream->flags |= _F_LINEBUF; }
    }

    stream->buf_pos = 0;
    stream->buf_end = 0;
    return 0;
}

void setbuf(FILE *stream, char *buf) { setvbuf(stream, buf, buf ? _IOFBF : _IONBF, BUFSIZ); }

int fflush(FILE *stream) {
    if (stream == NULL) {
        int ret = 0;
        if (stdout) ret |= fflush(stdout);
        if (stderr) ret |= fflush(stderr);
        return ret;
    }

    if (!(stream->flags & _F_WRITE)) { return 0; }
    if ((stream->flags & _F_NOBUF) ||  stream->buf_pos == 0) { return 0; }

    if (stream->buffer && stream->buf_pos > 0) {
        int written = _io_write(stream->fd, stream->buffer, (int)stream->buf_pos);
        if (written < 0) { stream->flags |= _F_ERR; return EOF; }
    }

    stream->buf_pos = 0;
    return 0;
}

/* ============================================================
 * Error handling
 * ============================================================
 */

void clearerr(FILE *stream) {
    if (stream) { stream->flags &= ~(_F_EOF | _F_ERR); }
}

int feof(FILE *stream) {
    return stream ? (stream->flags & _F_EOF) : 0;
}

int ferror(FILE *stream) {
    return stream ? (stream->flags & _F_ERR) : 0;
}

/* ============================================================
 * Unformatted input functions
 * ============================================================
 */

int fgetc(FILE *stream) {
    if (_ensure_init() != 0) { return EOF; }
    if (stream == NULL) { return EOF; }
    if (!(stream->flags & _F_READ)) { return EOF; }

    if (stream->ungetc_buf >= 0) {
        int c = stream->ungetc_buf;
        stream->ungetc_buf = -1;
        return c;
    }

    if (stream->flags & _F_EOF) { return EOF; }

    if (stream->flags & _F_NOBUF) {
        char c;
        int n = _io_read(stream->fd, &c, 1);
        if (n <= 0) { stream->flags |= _F_EOF; return EOF; }
        return (unsigned char)c;
    }

    if (stream->buf_pos >= stream->buf_end) {
        int n = _io_read(stream->fd, stream->buffer, (int)stream->buf_size);
        if (n <= 0) { stream->flags |= _F_EOF; return EOF; }
        stream->buf_pos = 0;
        stream->buf_end = (size_t)n;
    }

    return (unsigned char)stream->buffer[stream->buf_pos++];
}

int getc(FILE *stream) {
    return fgetc(stream);
}

int getchar(void) {
    return fgetc(stdin);
}

char *fgets(char *s, int n, FILE *stream) {
    if (_ensure_init() != 0) { return NULL; }
    if (s == NULL || n <= 0 || stream == NULL) { return NULL; }
    if (!(stream->flags & _F_READ)) { return NULL; }

    char *ptr = s;
    int count = 0;

    while (count < n - 1) {
        int c = fgetc(stream);
        if (c == EOF) { if (count == 0) { return NULL; } break; }
        *ptr++ = (char)c;
        count++;
        if (c == '\n') { break; }
    }

    *ptr = '\0';
    return s;
}

int ungetc(int c, FILE *stream) {
    if (_ensure_init() != 0) { return EOF; }
    if (stream == NULL) { return EOF; }
    if (c == EOF) { return EOF; }
    if (!(stream->flags & _F_READ)) { return EOF; }

    if (stream->ungetc_buf >= 0) { return EOF; }

    stream->ungetc_buf = (unsigned char)c;
    stream->flags &= ~_F_EOF;
    return (unsigned char)c;
}

/* ============================================================
 * Unformatted output functions
 * ============================================================
 */

int fputc(int c, FILE *stream) {
    if (_ensure_init() != 0) { return EOF; }
    if (stream == NULL) { return EOF; }
    if (!(stream->flags & _F_WRITE)) { return EOF; }

    unsigned char uc = (unsigned char)c;

    if (stream->flags & _F_NOBUF) {
        int n = _io_write(stream->fd, (const char*)&uc, 1);
        if (n <= 0) { stream->flags |= _F_ERR; return EOF; }
        return uc;
    }

    stream->buffer[stream->buf_pos++] = (char)uc;

    int should_flush = (stream->buf_pos >= stream->buf_size) || ((stream->flags & _F_LINEBUF) && uc == '\n');
    if (should_flush) { if (fflush(stream) == EOF) { return EOF; } }
    return uc;
}

int putc(int c, FILE *stream) {
    return fputc(c, stream);
}

int putchar(int c) {
    return fputc(c, stdout);
}

int fputs(const char *s, FILE *stream) {
    if (_ensure_init() != 0) { return EOF; }
    if (s == NULL || stream == NULL) { return EOF; }
    if (!(stream->flags & _F_WRITE)) { return EOF; }

    while (*s) { if (fputc(*s++, stream) == EOF) { return EOF; } }
    return 0;
}

int puts(const char *s) {
    if (_ensure_init() != 0) { return EOF; }
    if (s == NULL) { return EOF; }

    if (fputs(s, stdout) == EOF) { return EOF; }
    if (fputc('\n', stdout) == EOF) { return EOF; }
    return 0;
}
