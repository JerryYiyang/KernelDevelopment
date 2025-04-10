#include "printk.h"
#include "vga.h"
#include "string.h"

void print_char(char c) {
    VGA_display_char(c);
}

void print_str(const char *s) {
    if (s == NULL) {
        print_str("(null)");
        return;
    }
    VGA_display_str(s);
}

