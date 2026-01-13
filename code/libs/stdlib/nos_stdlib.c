#include "nos_stdlib.h"
#include "nos_string.h"
#include "nos_errno.h"
#include "nos_mem.h"
#include "syscall.h"

#ifndef NOT_YET_IMPLEMENTED
#define NOT_YET_IMPLEMENTED(func) PutString("Function " #func " is not yet implemented.\n", 39);
#endif

/* =============================================================
 * Internal state
 * =============================================================
 */
static int _mem_initialized = 0;

#define DEFAULT_HEAP_SIZE 4096

static int _ensure_mem_init(void) {
    if (_mem_initialized) { return 0; }
    if (mem_init(DEFAULT_HEAP_SIZE) == NULL) { return -1; }
    _mem_initialized = 1;
    return 0;
}

/* =============================================================
 * Process Control Functions
 * =============================================================
 */
void _exit(const int status) {
    Exit(status);
}

#define MAX_ATEXIT_HANDLERS 32

static void (*atexit_handlers[MAX_ATEXIT_HANDLERS])(void);
static int atexit_count = 0;

int atexit(void (*func)(void)) {
    if (atexit_count >= MAX_ATEXIT_HANDLERS) {
        return -1;
    }
    atexit_handlers[atexit_count++] = func;
    return 0;
}

void abort(void) {
    // TODO: Create a core dump or similar mechanism
    _exit(EXIT_FAILURE);
    __builtin_unreachable();
}

int exit(const int status) {
    for (int i = atexit_count - 1; i >= 0; i--) {
        atexit_handlers[i]();
    }

    // TODO: FLush all stdio buffers
    // TODO: Close stdio streams (buffers, files, etc.)
    // TODO: Free libc allocated memory

    _exit(status);
    __builtin_unreachable();
}

/* =============================================================
 * Memory Management Functions
 * =============================================================
 */
void* malloc(size_t size) {
    if (size == 0) { return NULL; }
    if (_ensure_mem_init() != 0) { return NULL; }
    return mem_alloc(size);
}

void free(void *ptr) {
    if (ptr == NULL) { return; }
    if (!_mem_initialized) { return; }
    mem_free(ptr);
}

void* calloc(size_t num, size_t size) {
    if (num == 0 || size == 0) { return NULL; }
    // Check for overflow
    if (size > (size_t)-1 / num) { return NULL; }
    if (_ensure_mem_init() != 0) { return NULL; }
    return mem_calloc(num, size);
}

void* realloc(void* ptr, size_t size) {
    if (ptr == NULL) { return malloc(size); }
    if (size == 0) { free(ptr); return NULL; }
    if (_ensure_mem_init() != 0) { return NULL; }
    return mem_realloc(ptr, size);
}

/* =============================================================
 * Numeric String Conversion Functions
 * =============================================================
 */
static int is_space(const char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

static int char_to_digit(const char c) {
    if (c >= '0' && c <= '9') { return c - '0'; }
    if (c >= 'a' && c <= 'z') { return c - 'a' + 10; }
    if (c >= 'A' && c <= 'Z') { return c - 'A' + 10; }
    return -1;
}

/*double atof(const char* nptr) {
    return strtol(nptr, NULL, 10);
}*/

int atoi(const char* str, const int base) {
    return (int)strtol(str, NULL, base);
}

long atol(const char* str, const int base) {
    return strtol(str, NULL, base);
}

long long atoll(const char* str, const int base) {
    return (long long)strtol(str, NULL, base);
}

long strtol(const char* str, char** endptr, int base) {
    const char* p = str;
    while (is_space(*p)) { p++; }

    int sign = 1;
    if (*p == '-') { sign = -1; p++; }
    else if (*p == '+') { p++; }

    if (base == 0) {
        if (*p == '0') {
            if (*(p + 1) == 'x' || *(p + 1) == 'X') { base = 16; p += 2; }
            else { base = 8; p++; }
        } else { base = 10; }
    } else if (base == 16) {
        if (*p == '0' && (*(p + 1) == 'x' || *(p + 1) == 'X')) { p += 2; }
    }

    long result = 0;
    int digit;
    while ((digit = char_to_digit(*p)) >= 0 && digit < base) {
        result = result * base + digit;
        p++;
    }

    if (endptr) {
        *endptr = (char*)p;
    }

    return sign * result;
}

long long strtoll(const char* str, char** endptr, int base) {
    return (long long)strtol(str, endptr, base);
}

unsigned long strtoul(const char* str, char** endptr, int base) {
    const char* p = str;
    while (is_space(*p)) { p++; }

    if (*p == '+') { p++; }

    if (base == 0) {
        if (*p == '0') {
            if (*(p + 1) == 'x' || *(p + 1) == 'X') { base = 16; p += 2; }
            else { base = 8; p++; }
        } else { base = 10; }
    } else if (base == 16) {
        if (*p == '0' && (*(p + 1) == 'x' || *(p + 1) == 'X')) { p += 2; }
    }

    unsigned long result = 0;
    int digit;
    while ((digit = char_to_digit(*p)) >= 0 && digit < base) {
        result = result * base + digit;
        p++;
    }

    if (endptr) {
        *endptr = (char*)p;
    }

    return result;
}

unsigned long long strtoull(const char* str, char** endptr, int base) {
    return (unsigned long long)strtoul(str, endptr, base);
}

float strtof(const char* str, char** endptr) {
    NOT_YET_IMPLEMENTED(strtof);
    if (endptr) { *endptr = (char*)str; }
    return 0.0f;
}

double strtod(const char* str, char** endptr) {
    NOT_YET_IMPLEMENTED(strtod);
    if (endptr) { *endptr = (char*)str; }
    return 0.0;
}

long double strtold(const char* str, char** endptr) {
    NOT_YET_IMPLEMENTED(strtold);
    if (endptr) { *endptr = (char*)str; }
    return 0.0L;
}

/* =============================================================
 * Miscellaneous Algorithms and Math Functions
 * =============================================================
 */
static unsigned int next_rand = 1;

// Linear congruential generator, very simple, but also very bad
int rand(void) {
    next_rand = next_rand * 1103515245 + 12345;
    return (unsigned int)(next_rand / 65536) % (RAND_MAX + 1);
}

void srand(const unsigned int seed) {
    next_rand = seed;
}

#define ABS_MACRO(type, name) \
type name(const type j) {      \
    return (j < 0) ? -j : j;   \
}

ABS_MACRO(int, abs)
ABS_MACRO(long int, labs)
ABS_MACRO(long long int, llabs)


void printf_simple(char* buf) {
    // TODO: Add support for format specifiers
    // TODO: Add buffering to optimize syscall number
    if (buf) { PutString(buf, strlen(buf)); }
}

void print_error(char* msg) {
    int err = __get_errno();
    // TODO: Optimize syscall number by using buffered and formattted output
    if (msg) { printf_simple(msg); }
    PutString(" (errno=", 8);
    PutInt(err);
    PutString(")\n", 2);
}

int scanf_simple(char *format, void *result){
    char tmp[255];
    int i;
    switch (format[1]){
        case 'd':
            if ( ( i = GetString(tmp, 255) )<  0 ){
                return -1;
            }
            *((int *) result) = (int) atoi(tmp, 10);
            return i;

        case 's':
            if ( ( i = GetString((char *) result, 255) )<  0 ){
                return -1;
            }
            ((char *)result)[i] = 0;
            return i;

        case 'c':
            if ( ( i = GetString(tmp, 255) ) != 1 ){
                return -1;
            }
            *((char *) result) = tmp[0];
            return 1;
        default:
            return -1;
    }
    return 0;
}

char* itoa(int value, char* str, int base) {
    static const char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char* ptr = str;
    int negative = 0;
    long val = value; // long to avoid overflow when value is INT_MIN

    if (base < 2 || base > 36) { *str = '\0'; return str; }
    if (value < 0 && base == 10) { negative = 1; val = -val; }

    do {
        long tmp_value = val;
        val /= base;
        *ptr++ = digits[tmp_value - val * base];
    } while (val);

    if (negative) { *ptr++ = '-'; }

    *ptr-- = '\0';

    char* reverse_ptr = str;
    while (reverse_ptr < ptr) {
        char tmp_char = *ptr;
        *ptr-- = *reverse_ptr;
        *reverse_ptr++ = tmp_char;
    }

    return str;
}
