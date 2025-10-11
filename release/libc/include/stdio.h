#ifndef _LIBC_STDIO_H
#define _LIBC_STDIO_H

#include <stddef.h>
#include <stdarg.h>

int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap);

int printf(const char *fmt, ...);
int printk(const char *fmt, ...);

int getline(char *buf, size_t size);

// void write(char* buffer, int len);
// void read(char* buffer, int size);

#endif // _LIBC_STDIO_H