#ifndef VGA_H
#define VGA_H

#include <stdint.h>

extern void VGA_clear(void);
extern void VGA_display_char(char c);
extern void VGA_display_str(const char *str);

#endif