#include "vga.h"
#include "string.h"
#include "printk.h"
#include "drivers.h"
#include "interrupts.h"
#include "serial.h"
#include "mmu.h"
#include "kmalloc.h"

// x86_64 is little endian

extern uint32_t multiboot_info_ptr;

void kmain(uint64_t multiboot_info) {
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
    MMU_init(multiboot_info);
    printk("MMU initialized\n");
    kmalloc_init();
    printk("Kmalloc initialized\n");

    printk("Testing kmalloc...\n");
    void *ptr1 = kmalloc(64);
    void *ptr2 = kmalloc(128);
    void *ptr3 = kmalloc(3000);
    printk("Allocated: ptr1=%p, ptr2=%p, ptr3=%p\n", ptr1, ptr2, ptr3);
    kmalloc_print_stats();
    printk("Freeing ptr1...\n");
    kfree(ptr1);
    printk("Freed ptr1 successfully\n");
    printk("Freeing ptr2...\n");
    kfree(ptr2);
    printk("Freed ptr2 successfully\n");
    printk("Freeing ptr3...\n");
    kfree(ptr3);
    printk("Freed ptr3 successfully\n");
    printk("Test completed");
    
    // Main system loop
    while (1) {
        __asm__ volatile("hlt");
    }
}