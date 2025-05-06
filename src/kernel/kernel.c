#include "vga.h"
#include "string.h"
#include "printk.h"
#include "drivers.h"
#include "interrupts.h"
#include "serial.h"

// x86_64 is little endian

void kmain(void) {
    VGA_clear();
    
    printk("Starting kernel\n");
    IRQ_init();
    printk("Interrupts initialized\n");
    SER_init();
    printk("Serial port initialized\n");
    ps2_init();
    printk("PS/2 controller initialized\n");
    IRQ_set_mask(1);
    kb_init();
    printk("Keyboard initialized\n");
    IRQ_clear_mask(1);
    
    while (1) {
        __asm__ volatile("hlt");
    }
}