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

/* Integer to string conversion */
char* itoa(int value, char* str, int base);

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

#endif //STD_LIB_C
