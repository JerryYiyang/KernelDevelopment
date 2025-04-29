#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#define NUM_IRQS 16
#include <stdint.h>

// handler registration
typedef void (*irq_handler_t)(int irq, int error_code, void* arg);

// IRQ table structure
static struct {
    void *arg;
    irq_handler_t handler;
} irq_table[NUM_IRQS];

// IDT entry structure
typedef struct {
    uint16_t isr_low;      // The lower 16 bits of the ISR's address
    uint16_t kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
    uint8_t ist;           // The IST in the TSS that the CPU will load into RSP; set to zero for now
    uint8_t attributes;    // Type and attributes; see the IDT page
    uint16_t isr_mid;      // The higher 16 bits of the lower 32 bits of the ISR's address
    uint32_t isr_high;     // The higher 32 bits of the ISR's address
    uint32_t reserved;     // Set to zero
} __attribute__((packed)) idt_entry_t;

// IDT pointer structure
typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idt_ptr_t;

// Interrupt frame structure
struct interrupt_frame {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

// PIC management
void PIC_remap(uint8_t offset1, uint8_t offset2);
void PIC_sendEOI(uint8_t irq);

// Interrupt handling functions
void IRQ_init(void);
void IRQ_set_mask(uint8_t irq);
void IRQ_clear_mask(uint8_t irq);
int IRQ_get_mask(int IRQline);
void IRQ_end_of_interrupt(int irq);
void interrupts_init(void);

// Global variables - use extern for definitions
extern idt_entry_t idt[256];
extern idt_ptr_t idtp;

// Handler registration
void IRQ_set_handler(int irq, irq_handler_t handler, void* arg);

// IDT management
void idt_init(void);
void idt_set_gate(uint8_t num, uint64_t handler, uint16_t selector, uint8_t ist, uint8_t type_attr);
extern void idt_load(void);

// Handler functions
//__attribute__((interrupt)) void interrupt_handler(struct interrupt_frame* frame);
void interrupt_handler(void);
__attribute__((noreturn)) void exception_handler(void);

// Simple test handler
extern void testirq(void);

#endif
// look into traps and interrupt gates