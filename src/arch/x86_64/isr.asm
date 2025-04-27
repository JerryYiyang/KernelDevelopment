global idt_load
extern idtp
extern isr_handler
extern irq_handler

section .text
bits 64

idt_load:
    lidt [idtp]
    ret

