#include "nos_ctype.h"

int isalnum(int c) {
    return (isalpha(c) || isdigit(c));
}

int isalpha(int c) {
    return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}

int isblank(int c) {
    return (c == ' ' || c == '\t');
}

int iscntrl(int c) {
    return (c >= 0 && c <= 31) || (c == 127);
}

int isdigit(int c) {
    return (c >= '0' && c <= '9');
}

int isgraph(int c) {
    return (c > 32 && c < 127);
}

int islower(int c) {
    return (c >= 'a' && c <= 'z');
}

int isprint(int c) {
    return (c >= 32 && c < 127);
}

int ispunct(int c) {
    return isprint(c) && !isalnum(c) && !isspace(c);
}

int isspace(int c) {
    return (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v');
}

int isupper(int c) {
    return (c >= 'A' && c <= 'Z');
}

int isxdigit(int c) {
    return (isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
}

int tolower(int c) {
    if (isupper(c)) {
        return c + ('a' - 'A');
    }
    return c;
}

int toupper(int c) {
    if (islower(c)) {
        return c - ('a' - 'A');
    }
    return c;
}