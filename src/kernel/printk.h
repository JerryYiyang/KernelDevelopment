#ifndef PRINTK_H
#define PRINTK_H

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

// integer limits
#define INT_MIN (-2147483647 - 1)
#define INT_MAX 2147483647
#define UINT_MAX 4294967295u
#define LONG_MIN (-9223372036854775807L - 1)
#define LONG_MAX 9223372036854775807L
#define ULONG_MAX 18446744073709551615UL
#ifndef UINTPTR_MAX
#define UINTPTR_MAX 18446744073709551615UL
#endif

void print_char(char c);
void print_str(const char *s);
void print_int(int n);
void print_uint(unsigned int n);
void print_hex(unsigned int n);
void print_ptr(void *p);
void print_short(short n);
void print_ushort(unsigned short n);
void print_short_hex(unsigned short n);
void print_long(long n);
void print_ulong(unsigned long n);
void print_long_hex(unsigned long n);

int printk(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

#endif