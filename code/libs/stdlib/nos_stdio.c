#include "nos_stdio.h"
#include "nos_stdlib.h"
#include "nos_string.h"
#include "nos_ctype.h"
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
static char _stdin_buffer[BUFSIZ];
static char _stdout_buffer[BUFSIZ];

static FILE _stdin_file = {
    STDIN_FILENO, 
    _stdin_buffer,
    BUFSIZ,
    0,
    0,
    _F_READ | _F_LINEBUF,
    -1
};

static FILE _stdout_file = {
    STDOUT_FILENO, 
    _stdout_buffer,
    BUFSIZ,
    0,
    0,
    _F_WRITE | _F_LINEBUF,
    -1
};

static FILE _stderr_file = {
    STDERR_FILENO, 
    NULL,
    0,
    0,
    0,
    _F_WRITE | _F_NOBUF,
    -1
};


FILE *stdin  = &_stdin_file;
FILE *stdout = &_stdout_file;
FILE *stderr = &_stderr_file;

static int _stdio_initialized = 0;

/* ============================================================
 * Initialization and cleanup
 * ============================================================
 */

int _stdio_init(void) {
    if (_stdio_initialized) { return 0; }
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
    if (_ensure_init() != 0) { return EOF; }
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
    if (_ensure_init() != 0) { return EOF; }
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
    if (_ensure_init() != 0) { return; }
    if (stream) { stream->flags &= ~(_F_EOF | _F_ERR); }
}

int feof(FILE *stream) {
    if (_ensure_init() != 0) { return EOF; }
    return stream ? (stream->flags & _F_EOF) : 0;
}

int ferror(FILE *stream) {
    if (_ensure_init() != 0) { return EOF; }
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

/* ============================================================
 * Format parsing and conversion helpers
 * ============================================================
 */

static inline int xdigit_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

static const char* parse_int(const char* s, int* value, int base, int* chars_read) {
    const char* start = s;
    int sign = 1;
    int val = 0;

    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') { s++; }

    if (base == 0) {
        if (*s == '0') {
            if (s[1] == 'x' || s[1] == 'X') { base = 16; s += 2; }
            else { base = 8; }
        }
        else { base = 10; }
    }
    else if (base == 16 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) { s += 2; }

    int digit_count = 0;
    while (*s) {
        int digit = -1;

        if (isdigit(*s)) { digit = *s - '0'; }
        else if (base == 16 && isxdigit(*s)) { digit = xdigit_val(*s); }

        if (digit < 0 || digit >= base) break;

        val = val * base + digit;
        digit_count++;
        s++;
    }

    if (digit_count == 0) return start;

    *value = sign * val;
    if (chars_read) *chars_read = (int)(s - start);
    return s;
}

static int uint_to_str(unsigned int value, char* buf, const int base, const int uppercase) {
    const char* digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    char tmp[32];
    int i = 0;

    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return 1;
    }

    while (value > 0) {
        tmp[i++] = digits[value % base];
        value /= base;
    }

    const int len = i;
    for (int j = 0; j < i; j++) { buf[j] = tmp[i - 1 - j]; }
    buf[len] = '\0';

    return len;
}

static int int_to_str(const int value, char* buf, const int base) {
    if (value < 0 && base == 10) {
        buf[0] = '-';
        uint_to_str((unsigned int)(-value), buf + 1, base, 0);
        return strlen(buf);
    }
    return uint_to_str((unsigned int)value, buf, base, 0);
}

/* ============================================================
 * Core formatting engine
 * ============================================================
 */

typedef struct {
    char* dest;
    size_t max_size;
    size_t written;
    FILE* stream;
    int error;
} printf_ctx_t;

static void putchar_ctx(printf_ctx_t* ctx, const char c) {
    if (ctx->error) { return; }

    if (ctx->dest) {
        if (ctx->written < ctx->max_size - 1) {
            ctx->dest[ctx->written] = c;
        }
    } else if (ctx->stream) {
        if (fputc(c, ctx->stream) == EOF) {
            ctx->error = 1;
            return;
        }
    }
    ctx->written++;
}

static void putstr_ctx(printf_ctx_t* ctx, const char* s) {
    while (*s) {
        putchar_ctx(ctx, *s++);
    }
}

static int do_vprintf(printf_ctx_t* ctx, const char* format, va_list args) {
    char buf[64];

    while (*format) {
        if (*format != '%') {
            putchar_ctx(ctx, *format++);
            continue;
        }

        format++;

        // Handle %%
        if (*format == '%') {
            putchar_ctx(ctx, '%');
            format++;
            continue;
        }

        // Parse width (simple version, no padding flags)
        int width = 0;
        while (isdigit(*format)) {
            width = width * 10 + (*format - '0');
            format++;
        }

        // Parse precision
        int precision = -1;
        if (*format == '.') {
            format++;
            precision = 0;
            while (isdigit(*format)) {
                precision = precision * 10 + (*format - '0');
                format++;
            }
        }

        // Parse conversion specifier
        switch (*format) {
            case 'd':
            case 'i': {
                int val = va_arg(args, int);
                int_to_str(val, buf, 10);
                putstr_ctx(ctx, buf);
                break;
            }
            case 'u': {
                unsigned int val = va_arg(args, unsigned int);
                uint_to_str(val, buf, 10, 0);
                putstr_ctx(ctx, buf);
                break;
            }
            case 'x': {
                unsigned int val = va_arg(args, unsigned int);
                uint_to_str(val, buf, 16, 0);
                putstr_ctx(ctx, buf);
                break;
            }
            case 'X': {
                unsigned int val = va_arg(args, unsigned int);
                uint_to_str(val, buf, 16, 1);
                putstr_ctx(ctx, buf);
                break;
            }
            case 'o': {
                unsigned int val = va_arg(args, unsigned int);
                uint_to_str(val, buf, 8, 0);
                putstr_ctx(ctx, buf);
                break;
            }
            case 'c': {
                char c = (char)va_arg(args, int);
                putchar_ctx(ctx, c);
                break;
            }
            case 's': {
                const char* s = va_arg(args, const char*);
                if (s == NULL) s = "(null)";

                if (precision >= 0) {
                    int len = 0;
                    while (s[len] && len < precision) { putchar_ctx(ctx, s[len++]); }
                } else { putstr_ctx(ctx, s); }
                break;
            }
            case 'p': {
                void* ptr = va_arg(args, void*);
                putstr_ctx(ctx, "0x");
                uint_to_str((unsigned int)ptr, buf, 16, 0);
                putstr_ctx(ctx, buf);
                break;
            }
            default:
                putchar_ctx(ctx, '%');
                putchar_ctx(ctx, *format);
                break;
        }

        format++;
    }

    if (ctx->dest && ctx->max_size > 0) {
        size_t pos = ctx->written < ctx->max_size ? ctx->written : ctx->max_size - 1;
        ctx->dest[pos] = '\0';
    }

    return ctx->error ? EOF : (int)ctx->written;
}

/* ============================================================
 * Printf family
 * ============================================================
 */

int vprintf(const char* format, va_list args) {
    if (_ensure_init() != 0) { return EOF; }
    if (format == NULL) { return EOF; }

    printf_ctx_t ctx = {0};
    ctx.stream = stdout;
    return do_vprintf(&ctx, format, args);
}

int vfprintf(FILE* stream, const char* format, va_list args) {
    if (_ensure_init() != 0) { return EOF; }
    if (stream == NULL || format == NULL) { return EOF; }

    printf_ctx_t ctx = {0};
    ctx.stream = stream;
    return do_vprintf(&ctx, format, args);
}

int vsprintf(char* s, const char* format, va_list args) {
    if (s == NULL || format == NULL) { return EOF; }

    printf_ctx_t ctx = {0};
    ctx.dest = s;
    ctx.max_size = (size_t)-1;
    return do_vprintf(&ctx, format, args);
}

int vsnprintf(char* s, size_t n, const char* format, va_list args) {
    if (s == NULL || format == NULL || n == 0) { return EOF; }

    printf_ctx_t ctx = {0};
    ctx.dest = s;
    ctx.max_size = n;
    return do_vprintf(&ctx, format, args);
}

int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vprintf(format, args);
    va_end(args);
    return ret;
}

int fprintf(FILE* stream, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfprintf(stream, format, args);
    va_end(args);
    return ret;
}

int sprintf(char* s, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vsprintf(s, format, args);
    va_end(args);
    return ret;
}

int snprintf(char* s, size_t n, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vsnprintf(s, n, format, args);
    va_end(args);
    return ret;
}

/* ============================================================
 * Core scanning engine
 * ============================================================ */

typedef struct {
    const char* src;
    FILE* stream;
    int chars_read;
    int conversions;
    int error;
} scanf_ctx_t;

static int getchar_ctx(scanf_ctx_t* ctx) {
    if (ctx->error) { return EOF; }

    int c;
    if (ctx->src) {
        c = *ctx->src;
        if (c == '\0') { return EOF; }
        ctx->src++;
    } else if (ctx->stream) {
        c = fgetc(ctx->stream);
        if (c == EOF) { ctx->error = 1; return EOF; }
    } else { return EOF; }

    ctx->chars_read++;
    return c;
}

static void ungetchar_ctx(scanf_ctx_t* ctx, int c) {
    if (c == EOF) return;

    if (ctx->src) { ctx->src--; }
    else if (ctx->stream) { ungetc(c, ctx->stream); }

    ctx->chars_read--;
}

static int do_vscanf(scanf_ctx_t* ctx, const char* format, va_list args) {
    while (*format) {
        if (*format == ' ' || *format == '\t' || *format == '\n') {
            format++;
            int c;
            while ((c = getchar_ctx(ctx)) != EOF) {
                if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
                    ungetchar_ctx(ctx, c);
                    break;
                }
            }
            continue;
        }

        if (*format != '%') {
            int c = getchar_ctx(ctx);
            if (c != *format) {
                if (c != EOF) { ungetchar_ctx(ctx, c); }
                return ctx->conversions;
            }
            format++;
            continue;
        }

        format++;

        if (*format == '%') {
            int c = getchar_ctx(ctx);
            if (c != '%') {
                if (c != EOF) { ungetchar_ctx(ctx, c); }
                return ctx->conversions;
            }
            format++;
            continue;
        }

        int suppress = 0;
        if (*format == '*') {
            suppress = 1;
            format++;
        }

        int width = 0;
        while (isdigit(*format)) {
            width = width * 10 + (*format - '0');
            format++;
        }
        if (width == 0) width = -1;

        switch (*format) {
            case 'd':
            case 'i': {
                int c;
                while ((c = getchar_ctx(ctx)) != EOF) {
                    if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
                        ungetchar_ctx(ctx, c);
                        break;
                    }
                }

                char buf[64];
                int i = 0;
                int base = (*format == 'i') ? 0 : 10;

                c = getchar_ctx(ctx);
                if (c == '-' || c == '+') {
                    if (width > 0 && i >= width) { ungetchar_ctx(ctx, c); break; }
                    buf[i++] = (char)c;
                    c = getchar_ctx(ctx);
                }

                if (base == 0 && c == '0') {
                    if (width > 0 && i >= width) { ungetchar_ctx(ctx, c); break; }
                    buf[i++] = (char)c;
                    c = getchar_ctx(ctx);
                    if (c == 'x' || c == 'X') {
                        if (width > 0 && i >= width) { ungetchar_ctx(ctx, c); break; }
                        buf[i++] = (char)c;
                        base = 16;
                    } else {
                        base = 8;
                        ungetchar_ctx(ctx, c);
                    }
                } else {
                    if (base == 0) { base = 10; }
                    ungetchar_ctx(ctx, c);
                }

                while ((c = getchar_ctx(ctx)) != EOF) {
                    int valid = 0;
                    if (base == 10 && isdigit((char)c)) { valid = 1; }
                    else if (base == 8 && c >= '0' && c <= '7') { valid = 1; }
                    else if (base == 16 && isxdigit((char)c)) { valid = 1; }

                    if (!valid || (width > 0 && i >= width)) {
                        ungetchar_ctx(ctx, c);
                        break;
                    }
                    buf[i++] = (char)c;
                }

                if (i == 0) { return ctx->conversions; }

                buf[i] = '\0';

                if (!suppress) {
                    int* ptr = va_arg(args, int*);
                    int value;
                    parse_int(buf, &value, base, NULL);
                    *ptr = value;
                    ctx->conversions++;
                }
                break;
            }
            case 'u':
            case 'o':
            case 'x':
            case 'X': {
                int c;
                while ((c = getchar_ctx(ctx)) != EOF) {
                    if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
                        ungetchar_ctx(ctx, c);
                        break;
                    }
                }

                int base = 10;
                if (*format == 'o') { base = 8; }
                else if (*format == 'x' || *format == 'X') { base = 16; }

                char buf[64];
                int i = 0;

                while ((c = getchar_ctx(ctx)) != EOF) {
                    int valid = 0;
                    if (base == 10 && isdigit((char)c)) { valid = 1; }
                    else if (base == 8 && c >= '0' && c <= '7') { valid = 1; }
                    else if (base == 16 && isxdigit((char)c)) { valid = 1; }

                    if (!valid || (width > 0 && i >= width)) {
                        ungetchar_ctx(ctx, c);
                        break;
                    }
                    buf[i++] = (char)c;
                }

                if (i == 0) { return ctx->conversions; }

                buf[i] = '\0';

                if (!suppress) {
                    unsigned int* ptr = va_arg(args, unsigned int*);
                    int value;
                    parse_int(buf, &value, base, NULL);
                    *ptr = (unsigned int)value;
                    ctx->conversions++;
                }
                break;
            }
            case 'c': {
                int count = (width > 0) ? width : 1;

                if (!suppress) {
                    char* ptr = va_arg(args, char*);
                    for (int i = 0; i < count; i++) {
                        int c = getchar_ctx(ctx);
                        if (c == EOF) { return ctx->conversions; }
                        ptr[i] = (char)c;
                    }
                    ctx->conversions++;
                } else {
                    for (int i = 0; i < count; i++) {
                        if (getchar_ctx(ctx) == EOF) { return ctx->conversions; }
                    }
                }
                break;
            }
            case 's': {
                int c;
                while ((c = getchar_ctx(ctx)) != EOF) {
                    if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
                        ungetchar_ctx(ctx, c);
                        break;
                    }
                }

                char* ptr = NULL;
                if (!suppress) { ptr = va_arg(args, char*); }

                int i = 0;
                while ((c = getchar_ctx(ctx)) != EOF) {
                    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') { ungetchar_ctx(ctx, c); break; }
                    if (width > 0 && i >= width) { ungetchar_ctx(ctx, c); break; }
                    if (ptr) { ptr[i] = (char)c; }
                    i++;
                }

                if (i == 0) return ctx->conversions;

                if (ptr) { ptr[i] = '\0'; ctx->conversions++; }
                break;
            }
            case 'p': {
                int c;
                while ((c = getchar_ctx(ctx)) != EOF) {
                    if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
                        ungetchar_ctx(ctx, c);
                        break;
                    }
                }

                char buf[64];
                int i = 0;

                c = getchar_ctx(ctx);
                if (c == '0') {
                    buf[i++] = (char)c;
                    c = getchar_ctx(ctx);
                    if (c == 'x' || c == 'X') { buf[i++] = (char)c; }
                    else { ungetchar_ctx(ctx, c); }
                }
                else { ungetchar_ctx(ctx, c); }

                while ((c = getchar_ctx(ctx)) != EOF) {
                    if (!isxdigit((char)c)) {
                        ungetchar_ctx(ctx, c);
                        break;
                    }
                    buf[i++] = (char)c;
                }

                if (i == 0) { return ctx->conversions; }

                buf[i] = '\0';

                if (!suppress) {
                    void** ptr = va_arg(args, void**);
                    int value;
                    parse_int(buf, &value, 16, NULL);
                    *ptr = (void*)value;
                    ctx->conversions++;
                }
                break;
            }
            default:
                return ctx->conversions;
        }

        format++;
    }

    return ctx->conversions;
}

/* ============================================================
 * Scanf family
 * ============================================================ */

int vscanf(const char* format, va_list args) {
    if (_ensure_init() != 0) { return EOF; }
    if (format == NULL) { return EOF; }

    scanf_ctx_t ctx = {0};
    ctx.stream = stdin;
    return do_vscanf(&ctx, format, args);
}

int vfscanf(FILE* stream, const char* format, va_list args) {
    if (_ensure_init() != 0) { return EOF; }
    if (stream == NULL || format == NULL) { return EOF; }

    scanf_ctx_t ctx = {0};
    ctx.stream = stream;
    return do_vscanf(&ctx, format, args);
}

int vsscanf(const char* s, const char* format, va_list args) {
    if (s == NULL || format == NULL) { return EOF; }

    scanf_ctx_t ctx = {0};
    ctx.src = s;
    return do_vscanf(&ctx, format, args);
}

int scanf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vscanf(format, args);
    va_end(args);
    return ret;
}

int fscanf(FILE* stream, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfscanf(stream, format, args);
    va_end(args);
    return ret;
}

int sscanf(const char* s, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vsscanf(s, format, args);
    va_end(args);
    return ret;
}