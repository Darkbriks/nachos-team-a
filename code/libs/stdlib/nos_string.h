#ifndef NOS_STRING_H
#define NOS_STRING_H

/*
 * @file nos_string.h
 * @brief Standard string and memory manipulation functions
 * Functions based on locale are not included
 * Excludes functions are strcoll and strxfrm
 */

#include "types.h"

/* ============================================================
 * Memory manipulation functions
 * ============================================================
 */

/**
 * @brief Copy bytes from src to dest until character c is found or n bytes are copied
 * @warning The behavior is undefined if the memory areas overlap
 * @param dest The destination buffer
 * @param src The source buffer
 * @param c The character to stop at
 * @param n The maximum number of bytes to copy
 * @return Pointer to the byte after c in dest, or NULL if c was not found in the first n bytes
 */
void* memccpy(void* dest, const void* src, int c, size_t n);

/**
 * @brief Locate the first occurrence of character c in the first n bytes of memory area s
 * @param s The memory area to search
 * @param c The character to search for
 * @param n The number of bytes to search
 * @return Pointer to the first occurrence of c in s, or NULL if not found
 */
void* memchr(const void* s, int c, size_t n);

/**
 * @brief Compare two memory regions
 * @param s1 The first memory region
 * @param s2 The second memory region
 * @param n The number of bytes to compare
 * @return 0 if equal, negative if s1 < s2, positive if s1 > s2
 */
int memcmp(const void* s1, const void* s2, size_t n);

/**
 * @brief Copy n bytes from src to dest
 * @warning The behavior is undefined if the memory areas overlap
 * @param dest The destination buffer
 * @param src The source buffer
 * @param n The number of bytes to copy
 * @return Pointer to the destination buffer
 */
void* memcpy(void* dest, const void* src, size_t n);

/**
 * @brief Move n bytes from src to dest, handling overlapping regions
 * @param dest The destination buffer
 * @param src The source buffer
 * @param n The number of bytes to move
 * @return Pointer to the destination buffer
 */
void* memmove(void* dest, const void* src, size_t n);

/**
 * @brief Set n bytes in dest to the value c
 * @param dest The destination buffer
 * @param c The value to set (interpreted as an unsigned char)
 * @param n The number of bytes to set
 * @return Pointer to the destination buffer
 */
void* memset(void* dest, int c, size_t n);

/* ============================================================
 * String manipulation functions
 * ============================================================
 */

/**
 * @brief Concatenate two strings
 * @warning Supposes dest has enough space to hold the result
 * @param dest The destination string
 * @param src The source string
 * @return Pointer to the destination string
 */
char* strcat(char* dest, const char* src);

/**
 * @brief Locate the first occurrence of character c in string str
 * @param str The input string
 * @param c The character to search for
 * @return Pointer to the first occurrence of c in str, or NULL if not found
 */
char* strchr(const char* str, int c);

/**
 * @brief Compare two strings
 * @warning Case sensitive
 * @warning Use ascii values for comparison
 * @param s1 The first string
 * @param s2 The second string
 * @return 0 if equal, negative if s1 < s2, positive if s1 > s2
 */
int strcmp(const char* s1, const char* s2);

/**
 * @brief Copy a string from src to dest
 * @warning Supposes dest has enough space to hold the result
 * @param dest The destination buffer
 * @param src The source string
 * @return Pointer to the destination buffer
 */
char* strcpy(char* dest, const char* src);

/**
 * @brief Calculate the length of the initial segment of s1 which does not contain any characters from s2
 * @param s1 The first string
 * @param s2 The second string
 * @return The length of the initial segment of s1 which does not contain any characters from s2
 */
size_t strcspn(const char* s1, const char* s2);

/**
 * @brief Duplicate a string by allocating new memory
 * @warning The caller is responsible for freeing the allocated memory
 * @warning Not yet implemented
 * @param s The input string
 * @return Pointer to the newly allocated string, or NULL on failure
 */
char* strdup(const char* s);

/**
 * @brief Get the error string corresponding to an error number
 * @warning Based on errors defined in syscall.h, not the same as standard C library
 * @param errnum The error number
 * @return Pointer to the error string
 */
char* strerror(int errnum);

/**
 * @brief Calculate the length of a string
 * @param str The input string
 * @return The length of the string (number of characters before the null terminator)
 */
size_t strlen(const char* str);

/**
 * @brief Concatenate up to n characters from src to dest
 * @warning Supposes dest has enough space to hold the result
 * @param dest The destination string
 * @param src The source string
 * @param n The maximum number of characters to concatenate
 * @return Pointer to the destination string
 */
char* strncat(char* dest, const char* src, size_t n);

/**
 * @brief Compare up to n characters of two strings
 * @warning Case sensitive
 * @warning Use ascii values for comparison
 * @param s1 The first string
 * @param s2 The second string
 * @param n The maximum number of characters to compare
 * @return 0 if equal, negative if s1 < s2, positive if s1 > s2
 */
int strncmp(const char* s1, const char* s2, size_t n);

/**
 * @brief Copy up to n characters from src to dest
 * @warning Supposes dest has enough space to hold the result
 * @warning If src is less than n characters long, the remainder of dest is filled with null bytes
 * @warning If src is longer than n characters, dest will not be null-terminated (i.e., it's the caller's responsibility to ensure null-termination if needed)
 * @param dest The destination buffer
 * @param src The source string
 * @param n The maximum number of characters to copy
 * @return Pointer to the destination buffer
 */
char* strncpy(char* dest, const char* src, size_t n);

/**
 * @brief Duplicate up to n characters of a string by allocating new memory
 * @warning The caller is responsible for freeing the allocated memory
 * @warning Not yet implemented
 * @param s The input string
 * @param n The maximum number of characters to duplicate
 * @return Pointer to the newly allocated string, or NULL on failure
 */
char* strndup(const char* s, size_t n);

/**
 * @brief Locate the first occurrence in s1 of any character from s2
 * @param s1 The string to search
 * @param s2 The string containing characters to search for
 * @return Pointer to the first occurrence in s1 of any character from s2, or NULL if none found
 */
char* strpbrk(const char* s1, const char* s2);

/**
 * @brief Locate the last occurrence of character c in string str
 * @param str The input string
 * @param c The character to search for
 * @return Pointer to the last occurrence of c in str, or NULL if not found
 */
char* strrchr(const char* str, int c);

/**
 * @brief Calculate the length of the initial segment of s1 which consists entirely of characters from s2
 * @param s1 The first string
 * @param s2 The second string
 * @return The length of the initial segment of s1 which consists entirely of characters from s2
 */
size_t strspn(const char* s1, const char* s2);

/**
 * @brief Locate the first occurrence of substring substr in string fullstr
 * @param fullstr The string to search
 * @param substr The substring to search for
 * @return Pointer to the first occurrence of substr in fullstr, or NULL if not found
 */
char* strstr(const char* fullstr, const char* substr);

/**
 * @brief Tokenize a string using given delimiters
 * @warning The first call should provide the string to tokenize; subsequent calls should provide NULL
 * @warning The function modifies the input string
 * @warning Not thread-safe due to use of static internal state
 * @param str The string to tokenize (or NULL for subsequent calls)
 * @param delim The string containing delimiter characters
 * @return Pointer to the next token, or NULL if no more tokens are found
 */
char* strtok(char* str, const char* delim);

#endif // NOS_STRING_H