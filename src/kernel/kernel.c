#include "vga.h"
#include "string.h"

void kmain(void) {
    VGA_display_char('H');
    VGA_display_char('e');
    VGA_display_char('l');
    VGA_display_char('l');
    VGA_display_char('o');
    VGA_display_char('\n');

        VGA_display_str("Welcome to my kernel!\n");

    while (1) {
        __asm__ volatile("hlt");
    }
}