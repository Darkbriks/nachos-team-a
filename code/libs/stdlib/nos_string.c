#include "nos_string.h"
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
    // TODO: Needs malloc
    NOT_YET_IMPLEMENTED(strdup);
    return NULL;
}

char* strerror(const int errnum) {
    // TODO: MaJ
    switch (errnum) {
        case  0: return "No error";
        case  1: return "Invalid argument";
        case  2: return "Bad address or memory access error";
        case  3: return "Arithmetic overflow";
        case  4: return "I/O error";
        case  5: return "Invalid format";
        case  6: return "End of file";
        case  7: return "Out of memory";
        case  8: return "Result out of range";
        case  9: return "No such process";
        case 10: return "Allocation table full";
        case 11: return "No such file or directory or table entry";
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
    // TODO: Needs malloc
    NOT_YET_IMPLEMENTED(strndup);
    return NULL;
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