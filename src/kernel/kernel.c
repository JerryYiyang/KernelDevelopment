#include "vga.h"
#include "string.h"
#include "printk.h"
#include "drivers.h"
#include "interrupts.h"

// x86_64 is little endian

void kmain(void) {
    VGA_clear();
    
    printk("start\n");
    ps2_init();
    printk("ps2 init done\n");
    kb_init();
    // printk("keyboard init done\n");
    // kb_polling();



    while (1) {
        __asm__ volatile("hlt");
    }
}

// call asm halt instruction to check for keyboard interrupts