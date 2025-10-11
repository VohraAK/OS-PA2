#ifndef _LIB_STRING_H
#define _LIB_STRING_H

#include "sys/cdefs.h"
#include <stdint.h>

/**
 * @brief Expects a string of characters (null-terminated), and returns the length of the string.
 * 
 * @param str Constant string whose length is needed.
 * @return uint32_t 4 byte length return value
 */
uint32_t strlen(const char* str);

/**
 * @brief Compares two strings and returns 0 if they are equal, a positive value if the first string is greater, and a negative value if the second string is greater.
 * 
 * @param str1 First string to compare
 * @param str2 Second string to compare
 * @return int Returns 0 if equal, positive if str1 > str2, negative if str1 < str2
 */
int strcmp(const char* str1, const char* str2);

char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, uint32_t n);
char* strtok(char* str, const char* delim);
char* strchr(const char* str, int c);
long  strtol(const char* str, char** endptr, int base);
char* strcat(char* dest, const char* src);

/* Re-entrant version of strtok, tokenizes str in place. */

char* strtok_r(char* str, const char* delim, char** saveptr);


/**
 * @brief Sets the first `n` bytes of the memory area pointed to by `dest` to the specified value `c`.
 * 
 * @param dest The destination buffer that needs to be filled.
 * @param c The value to set in each byte of the destination buffer. It is passed as an `int`, but the function will convert it to an `unsigned char` before setting.
 * @param n The number of bytes to set in the destination buffer.
 * @return void* Returns a pointer to the destination buffer `dest`.
 */
void* memset(void* dest, int c, uint32_t n);

/**
 * @brief Copies `n` bytes from the source memory area pointed to by `src` to the destination memory area pointed to by `dest`.
 * 
 * @param dest The destination buffer where the data will be copied to.
 * @param src The source buffer from which the data will be copied.
 * @param n The number of bytes to copy from the source to the destination buffer.
 * @return void* Returns a pointer to the destination buffer `dest`.
 */
void* memcpy(void* dest, const void* src, uint32_t n);

int32_t memcmp(const void* s1, const void* s2, uint32_t n);

#endif // _LIB_STRING_H