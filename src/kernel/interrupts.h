#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

typedef void (*irq_handler_t)(int irq, int error_code, void* arg);

struct {
    void *arg;
    irq_handler_t handler;
} irq_table[16];

typedef struct {
    uint16_t isr_low;      // the lower 16 bits of the ISR's address
    uint16_t kernel_cs;    // the GDT segment selector that the CPU will load into CS before calling the ISR
    uint8_t ist;           // the IST in the TSS that the CPU will load into RSP; set to zero for now
    uint8_t attributes;    // type and attributes; see the IDT page
    uint16_t isr_mid;      // the higher 16 bits of the lower 32 bits of the ISR's address
    uint32_t isr_high;     // the higher 32 bits of the ISR's address
    uint32_t reserved;     // set to zero
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idt_ptr_t;

struct interrupt_frame {
    // pushed by common handler
    uint64_t ds;
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    
    // pushed by ISR macros
    uint64_t int_no;
    uint64_t err_code;
    
    // pushed by CPU
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

// PIC management
void PIC_remap(uint8_t offset1, uint8_t offset2);
void PIC_sendEOI(uint8_t irq);

// interrupt handling functions
void IRQ_init(void);
void IRQ_set_mask(uint8_t irq);
void IRQ_clear_mask(uint8_t irq);
int IRQ_get_mask(int IRQline);
void IRQ_end_of_interrupt(int irq);
void interrupts_init(void);

extern idt_entry_t idt[256];
extern idt_ptr_t idtp;
extern struct tss_struct tss;

void IRQ_set_handler(int irq, irq_handler_t handler, void* arg);
void idt_init(void);
void idt_set_gate(uint8_t num, uint64_t handler, uint16_t selector, uint8_t ist, uint8_t type_attr);
extern void idt_load(void);
void setup_tss(void);
void interrupt_handler(struct interrupt_frame* frame);
extern void default_interrupt(void);

// isrs
extern void testirq(void);
extern void isr_0(void);
extern void isr_1(void);
extern void isr_2(void);
extern void isr_3(void);
extern void isr_4(void);
extern void isr_5(void);
extern void isr_6(void);
extern void isr_7(void);
extern void isr_8(void);
extern void isr_9(void);
extern void isr_10(void);
extern void isr_11(void);
extern void isr_12(void);
extern void isr_13(void);
extern void isr_14(void);
extern void isr_15(void);
extern void isr_16(void);
extern void isr_17(void);
extern void isr_18(void);
extern void isr_19(void);
extern void isr_20(void);
extern void isr_21(void);
extern void isr_22(void);
extern void isr_23(void);
extern void isr_24(void);
extern void isr_25(void);
extern void isr_26(void);
extern void isr_27(void);
extern void isr_28(void);
extern void isr_29(void);
extern void isr_30(void);
extern void isr_31(void);
extern void isr_32(void);
extern void isr_33(void);
extern void isr_34(void);
extern void isr_35(void);
extern void isr_36(void);
extern void isr_37(void);
extern void isr_38(void);
extern void isr_39(void);
extern void isr_40(void);
extern void isr_41(void);
extern void isr_42(void);
extern void isr_43(void);
extern void isr_44(void);
extern void isr_45(void);
extern void isr_46(void);
extern void isr_47(void);

#endif