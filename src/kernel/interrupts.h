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

struct idt_entry {
    uint16_t target_offset_low;     // Target Offset [15:0] - Low part of handler address
    uint16_t target_selector;       // Target Selector - Kernel code segment selector
    uint8_t ist;                    // IST
    uint8_t type_attr;              // Type and attributes (includes P bit and DPL)
    uint16_t target_offset_middle;  // Target Offset [31:16] - Middle part of handler address
    uint32_t target_offset_high;    // Target Offset [63:32] - High part of handler address
    uint32_t reserved;              // Reserved, should be 0
} __attribute__((packed));

extern void idt_set_gate(uint8_t num, uint64_t handler, uint16_t selector, uint8_t ist, uint8_t type_attr);

#endif