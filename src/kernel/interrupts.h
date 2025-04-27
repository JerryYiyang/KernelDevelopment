#ifndef INTERRUPTS_H
#define INTERRUPTS_H
#define NUM_IRQS 256
#include <stdint.h>

extern void IRQ_init(void);
extern void IRQ_set_mask(uint8_t irq);
extern void IRQ_clear_mask(uint8_t irq);
extern int IRQ_get_mask(int IRQline);
extern void IRQ_end_of_interrupt(int irq);

typedef void (*irq_handler_t)(int, int, void*);
static struct {
    void *arg;
    irq_handler_t handler;
} irq_table[NUM_IRQS];

extern void IRQ_set_handler(int irq, irq_handler_t handler, void *arg);

typedef struct {
	uint16_t    isr_low;      // The lower 16 bits of the ISR's address
	uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
	uint8_t	    ist;          // The IST in the TSS that the CPU will load into RSP; set to zero for now
	uint8_t     attributes;   // Type and attributes; see the IDT page
	uint16_t    isr_mid;      // The higher 16 bits of the lower 32 bits of the ISR's address
	uint32_t    isr_high;     // The higher 32 bits of the ISR's address
	uint32_t    reserved;     // Set to zero
} __attribute__((packed)) idt_entry_t;

struct idt_ptr {
    uint16_t limit;       // Size of idt - 1
    uint32_t base;        // Base address of the IDT
} __attribute__((packed));

extern struct idt_entry idt[256];
extern struct idt_ptr idtp;

extern void idt_set_gate(uint8_t num, uint64_t handler, uint16_t selector, uint8_t ist, uint8_t type_attr);
extern void interrupt_handler(void);

#endif


// look into traps and interrupt gates