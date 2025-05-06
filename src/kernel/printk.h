#ifndef PRINTK_H
#define PRINTK_H

#include <stdarg.h>
#include <stdint.h>

// integer limits
#define INT_MIN (-2147483647 - 1)
#define INT_MAX 2147483647
#define UINT_MAX 4294967295u
#define LONG_MIN (-9223372036854775807L - 1)
#define LONG_MAX 9223372036854775807L
#ifndef ULONG_MAX
#define ULONG_MAX 18446744073709551615UL
#endif
#ifndef UINTPTR_MAX
#define UINTPTR_MAX 18446744073709551615UL
#endif

extern void print_char(char c);
extern void print_str(const char *s);
extern int printk(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
extern void ser_print_char(char c);
extern void ser_print_str(const char *s);

#endif