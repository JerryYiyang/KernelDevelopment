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
    printk("keyboard init done\n");
    kb_polling();

    // interrupt init
    cli();
    PIC_remap(0x20, 0x28);
    struct idt_entry idt[256];
    struct idt_ptr   idtp;
    sti();

    while (1) {
        __asm__ volatile("hlt");
    }
}