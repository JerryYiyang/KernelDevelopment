#include "vga.h"
#include "string.h"
#include "printk.h"
#include "ps2.h"

// x86_64 is little endian

void kmain(void) {
    VGA_clear();

    ps2_init();
    kb_init();

    while (1) {
        __asm__ volatile("hlt");
    }
}