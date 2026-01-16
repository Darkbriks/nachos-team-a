#ifndef STD_LIB_C
#define STD_LIB_C

#include "nos_stddef.h"
#include "types.h"

/* ============================================================
 * Constants
 * ============================================================
 */

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#ifndef RAND_MAX
#define RAND_MAX 32767
#endif

/* ============================================================
 * Utility macros
 * ============================================================
 */
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

/* =============================================================
 * Process Control Functions
 * =============================================================
 */
int atexit(void (*func)(void));
void abort(void) NORETURN;
int exit(int status) NORETURN;

/* =============================================================
 * Memory Management Functions
 * =============================================================
 */
void* malloc(size_t size);
void free(void *ptr);
void* calloc(size_t num, size_t size);
void* realloc(void* ptr, size_t size);

/* =============================================================
 * Numeric String Conversion Functions
 * =============================================================
 */
// double atof(const char* nptr);
int atoi(const char* str, int base);
long atol(const char* str, int base);
long long atoll(const char* str, int base);
long strtol(const char* str, char** endptr, int base);
long long strtoll(const char* str, char** endptr, int base);
unsigned long strtoul(const char* str, char** endptr, int base);
unsigned long long strtoull(const char* str, char** endptr, int base);
float strtof(const char* str, char** endptr);
double strtod(const char* str, char** endptr);
long double strtold(const char* str, char** endptr);

/* =============================================================
 * Miscellaneous Algorithms and Math Functions
 * =============================================================
 */
int rand(void);
void srand(unsigned int seed);

int abs(int j);
long int labs(long int j);
long long int llabs(long long int j);

// TODO: Missing functions that can be fun to implement :
// qsort, bsearch, div, ldiv, lldiv
// See https://en.cppreference.com/w/cpp/header/cstdlib.html

/* =============================================================
 * I/O Functions
 * =============================================================
 */

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

#endif //STD_LIB_C
