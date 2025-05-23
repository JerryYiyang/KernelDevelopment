#ifndef STRING_H
#define STRING_H

#include <stddef.h>

extern void *memset(void *dst, int c, size_t n);
extern void *memcpy(void *dest, const void *src, size_t n);
extern size_t strlen(const char *s);
extern char *strcpy(char *dest, const char *src);
extern int strcmp(const char *s1, const char *s2);
extern const char *strchr(const char *s, int c);
extern char *strdup(const char *s);

#endif