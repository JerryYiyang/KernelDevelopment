#ifndef VGA_H
#define VGA_H

#include <stdint.h>

void VGA_clear(void);
void VGA_display_char(char c);
void VGA_display_str(const char *str);

#endif