#include "nos_string.h"

#include "nos_errno.h"
#include "nos_mem.h"
#include "nos_stddef.h"
#include "types.h"
#include "syscall.h"

#ifndef NOT_YET_IMPLEMENTED
#define NOT_YET_IMPLEMENTED(func) PutString("Function " #func " is not yet implemented.\n", 60);
#endif

void* memccpy(void* dest, const void* src, int c, size_t n) {
    size_t i;
    unsigned char uc = (unsigned char)c;

    for (i = 0; i < n; i++) {
        ((unsigned char*)dest)[i] = ((const unsigned char*)src)[i];
        if (((const unsigned char*)src)[i] == uc) {
            return (char*)dest + i + 1;
        }
    }
    return NULL;
}

void* memchr(const void* s, int c, size_t n) {
    size_t i;
    unsigned char uc = (unsigned char)c;

    for (i = 0; i < n; i++) {
        if (((const unsigned char*)s)[i] == uc) {
            return (void*)((const unsigned char*)s + i);
        }
    }
    return NULL;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    size_t i;
    for (i = 0; i < n; i++) {
        unsigned char byte1 = ((const unsigned char*)s1)[i];
        unsigned char byte2 = ((const unsigned char*)s2)[i];
        if (byte1 != byte2) {
            return (int)byte1 - (int)byte2;
        }
    }
    return 0;
}

void* memcpy(void* dest, const void* src, size_t n) {
    size_t i = 0;

    while (i < n && (((uintptr_t)((char*)dest + i)) & 7) != 0) {
        ((char*)dest)[i] = ((const char*)src)[i];
        i++;
    }

    for (; i + sizeof(uint64_t) <= n; i += sizeof(uint64_t)) {
        *(uint64_t*)((char*)dest + i) = *(const uint64_t*)((const char*)src + i);
    }

    for (; i + sizeof(uint16_t) <= n; i += sizeof(uint16_t)) {
        *(uint16_t*)((char*)dest + i) = *(const uint16_t*)((const char*)src + i);
    }

    for (; i < n; i++) { ((char*)dest)[i] = ((const char*)src)[i]; }

    return dest;
}

void* memmove(void* dest, const void* src, size_t n) {
    if (dest < src || (char*)dest >= (char*)src + n) {
        return memcpy(dest, src, n);
    } else {
        char* ptr_d = dest;
        const char* ptr_s = src;
        ptr_d += n;
        ptr_s += n;
        for (size_t i = 0; i < n; i++) {
            *(--ptr_d) = *(--ptr_s);
        }
        return dest;
    }
}

void* memset(void* dest, int c, size_t n) {
    size_t i = 0; unsigned char uc = (unsigned char)c;

    while (i < n && (((uintptr_t)((char*)dest + i)) & 7) != 0) {
        ((char*)dest)[i++] = uc;
    }

    uint64_t pattern64 = (uint64_t)uc * 0x0101010101010101ULL;
    for (; i + sizeof(uint64_t) <= n; i += sizeof(uint64_t)) {
        *(uint64_t*)((char*)dest + i) = pattern64;
    }

    uint16_t pattern16 = (uint16_t)uc * 0x0101;
    for (; i + sizeof(uint16_t) <= n; i += sizeof(uint16_t)) {
        *(uint16_t*)((char*)dest + i) = pattern16;
    }

    for (; i < n; i++) { ((char*)dest)[i] = uc; }
    return dest;
}

char* strcat(char* dest, const char* src) {
    char* ptr = dest;
    while (*ptr != '\0') { ptr++; }
    while (*src != '\0') { *ptr++ = *src++; }
    *ptr = '\0';
    return dest;
}

char* strchr(const char* str, int c) {
    unsigned char uc = (unsigned char)c;
    while (*str != '\0') {
        if (*str == uc) {
            return (char*)str;
        }
        str++;
    }
    if (uc == '\0') {
        return (char*)str;
    }
    return NULL;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 != '\0' && *s1 == *s2) {
        s1++;
        s2++;
    }
    return (unsigned char)(*s1) - (unsigned char)(*s2);
}

char* strcpy(char* dest, const char* src) {
    char* ptr = dest;
    while (*src != '\0') {
        *ptr++ = *src++;
    }
    *ptr = '\0';
    return dest;
}

size_t strcspn(const char* str, const char* reject) {
    size_t len = 0;
    while (str[len] != '\0') {
        const char* r = reject;
        while (*r != '\0') {
            if (str[len] == *r) {
                return len;
            }
            r++;
        }
        len++;
    }
    return len;
}

char* strdup(const char* s) {
    if (s == NULL) { return NULL; }

    const size_t len = strlen(s) + 1; // +1 pour le '\0'
    char* dup = (char*)mem_alloc(len);

    if (dup == NULL) { return NULL; }

    memcpy(dup, s, len);
    return dup;
}

char* strerror(const int errnum) {
    switch (errnum) {
        case  E_SUCCESS: return "No error";
        case  E_INVAL: return "Invalid argument";
        case  E_FAULT: return "Bad address / memory access error";
        case  E_OVERFLOW: return "Arithmetic overflow";
        case  E_IO: return "I/O error";
        case  E_FORMAT: return "Invalid format";
        case  E_EOF: return "End of file";
        case  E_NOMEM: return "Out of memory";
        case  E_RANGE: return "Result out of range";
        case  E_NOSPC: return "No such process";
        case  E_FTABLE: return "Allocation table full (file table, semaphore table, etc.)";
        case  E_NOENT: return "No such file or directory or table entry";
        case  E_NOCPC: return "Not a child process";
        case  E_THREAD_LIMIT: return "Maximum threads reached";
        case  E_STACK_ADDR: return "Invalid stack address";
        case  E_BUSY: return "Resource busy";
        case  E_AGAIN: return "Resource temporarily unavailable";
        case  E_DOM: return "Math argument out of domain";
        case  E_ILSEQ: return "Illegal byte sequence";
        case  E_PERM: return "Operation not permitted";
        case  E_ACCES: return "Permission denied";
        case  E_EXIST: return "File exists";
        case  E_NOSYS: return "Function not implemented";
        case  E_NOTDIR: return "Not a directory";
        case  E_ISDIR: return "Is a directory";
        case  E_BADF: return "Bad file descriptor";

        // TODO: Uncomment on merge with network
        /*case  E_REFUSED: return "Connection refused";
        case  E_NOTCONN: return "Not connected";
        case  E_ADDRINUSE: return "Address/port already in use";
        case  E_PIPE: return "Broken pipe (peer closed)";
        case  E_CONNRESET: return "Connection reset by peer";
        case  E_TIMEOUT: return "Connection timed out";
        case  E_CLOSED: return "Connection closed";
        case  E_NOPORT: return "No available port";
        case  E_NOAVAILCONN: return "No available connection";
        case  E_WOULDBLOCK: return "Operation would block (non-blocking mode)";*/
        default: return "Unknown error";
    }
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

char* strncat(char* dest, const char* src, size_t n) {
    char* ptr = dest;
    while (*ptr != '\0') { ptr++; }
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        *ptr++ = src[i];
    }
    *ptr = '\0';
    return dest;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    size_t i;
    for (i = 0; i < n; i++) {
        if (s1[i] != s2[i]) {
            return (unsigned char)(s1[i]) - (unsigned char)(s2[i]);
        }
        if (s1[i] == '\0') {
            return 0;
        }
    }
    return 0;
}

char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

char* strndup(const char* s, size_t n) {
    if (s == NULL) { return NULL; }

    size_t len = 0;
    while (len < n && s[len] != '\0') { len++; }

    char* dup = (char*)mem_alloc(len + 1);
    if (dup == NULL) { return NULL; }

    memcpy(dup, s, len);
    dup[len] = '\0';
    return dup;
}

char* strpbrk(const char* s1, const char* s2) {
    while (*s1 != '\0') {
        const char* p = s2;
        while (*p != '\0') {
            if (*s1 == *p) {
                return (char*)s1;
            }
            p++;
        }
        s1++;
    }
    return NULL;
}

char* strrchr(const char* str, int c) {
    const char* last_occurrence = NULL;
    unsigned char uc = (unsigned char)c;

    while (*str != '\0') {
        if (*str == uc) {
            last_occurrence = str;
        }
        str++;
    }
    if (uc == '\0') {
        return (char*)str;
    }
    return (char*)last_occurrence;
}

size_t strspn(const char* str, const char* accept) {
    size_t len = 0;
    while (str[len] != '\0') {
        const char* a = accept;
        int found = 0;
        while (*a != '\0') {
            if (str[len] == *a) {
                found = 1;
                break;
            }
            a++;
        }
        if (!found) {
            return len;
        }
        len++;
    }
    return len;
}

char* strstr(const char* fullstr, const char* substr) {
    if (*substr == '\0') {
        return (char*)fullstr;
    }

    while (*fullstr != '\0') {
        const char* f = fullstr;
        const char* s = substr;

        while (*f != '\0' && *s != '\0' && *f == *s) {
            f++;
            s++;
        }

        if (*s == '\0') {
            return (char*)fullstr;
        }

        fullstr++;
    }
    return NULL;
}

char* strtok(char* str, const char* delim) {
    static char* last = NULL;
    if (str == NULL) {
        if (last == NULL) {
            return NULL;
        }
        str = last;
    }

    while (*str != '\0' && strchr(delim, *str) != NULL) {
        str++;
    }

    if (*str == '\0') {
        last = str;
        return NULL;
    }

    char* token_start = str;

    while (*str != '\0' && strchr(delim, *str) == NULL) {
        str++;
    }

    if (*str != '\0') {
        *str = '\0';
        last = str + 1;
    } else {
        last = str;
    }

    return token_start;
}