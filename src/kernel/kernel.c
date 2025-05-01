#include "vga.h"
#include "string.h"
#include "printk.h"
#include "drivers.h"
#include "interrupts.h"

// x86_64 is little endian

void kmain(void) {
    VGA_clear();
    
    printk("Starting kernel\n");
    IRQ_init();
    printk("Interrupts initialized\n");
    printk("Testing software interrupt (int $0x20)...\n");
    __asm__ volatile("int $0x20");
    printk("Software interrupt test passed\n");
    ps2_init();
    printk("PS/2 controller initialized\n");
    kb_init();
    printk("Keyboard initialized\n");
    IRQ_clear_mask(1);
    printk("Keyboard interrupt enabled\n");
    
    while (1) {
        __asm__ volatile("hlt");
    }
}