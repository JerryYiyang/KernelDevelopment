#include "vga.h"
#include "string.h"
#include "printk.h"
#include "drivers.h"
#include "interrupts.h"
#include "serial.h"
#include "mmu.h"

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
    printk("Virtual memory initialized (by boot.asm)\n");
    printk("Testing virtual memory functions\n");
    printk("Test 1: MMU_alloc_page and MMU_free_page\n");
    void* single_page = MMU_alloc_page();
    if (single_page) {
        printk("Successfully allocated page at virtual address %p\n", single_page);
        *((uint32_t*)single_page) = 0xDEADBEEF;
        printk("Successfully wrote 0xDEADBEEF to allocated page\n");
        uint32_t read_val = *((uint32_t*)single_page);
        printk("Read back value: 0x%x %s\n", read_val, 
              (read_val == 0xDEADBEEF) ? "(correct)" : "(ERROR)");
        MMU_free_page(single_page);
        printk("Successfully freed page\n");
    } else {
        printk("ERROR: Failed to allocate page\n");
    }
    printk("Test 2: MMU_alloc_pages and MMU_free_pages\n");
    const int NUM_PAGES = 4;
    void* multi_pages = MMU_alloc_pages(NUM_PAGES);
    if (multi_pages) {
        printk("Successfully allocated %d pages starting at virtual address %p\n", 
              NUM_PAGES, multi_pages);
        for (int i = 0; i < NUM_PAGES; i++) {
            uint32_t* page_addr = (uint32_t*)((uint64_t)multi_pages + i * PAGE_SIZE);
            *page_addr = 0xCAFE0000 + i;
            printk("Wrote 0x%x to page %d at address %p\n", 0xCAFE0000 + i, i, page_addr);
        }
        int errors = 0;
        for (int i = 0; i < NUM_PAGES; i++) {
            uint32_t* page_addr = (uint32_t*)((uint64_t)multi_pages + i * PAGE_SIZE);
            uint32_t expected = 0xCAFE0000 + i;
            uint32_t actual = *page_addr;
            if (actual != expected) {
                printk("ERROR: Page %d contains 0x%x, expected 0x%x\n", 
                      i, actual, expected);
                errors++;
            }
        }
        if (errors == 0) {
            printk("All pages verified successfully\n");
        }
        MMU_free_pages(multi_pages, NUM_PAGES);
        printk("Successfully freed all %d pages\n", NUM_PAGES);
    } else {
        printk("ERROR: Failed to allocate multiple pages\n");
    }
    printk("Virtual mem test complete\n");
    
    // Main system loop
    while (1) {
        __asm__ volatile("hlt");
    }
}