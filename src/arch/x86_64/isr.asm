; isr.asm - Simple test interrupt handler for x86_64

section .text
bits 64

; Export the test handler and IDT load function
global idt_load
global testirq

; External reference to IDT pointer
extern idtp

; Function to load the IDT
idt_load:
    lidt [idtp]        ; Load IDT with the IDT pointer
    ret

; Test IRQ handler that just returns
align 16
testirq:
    iretq              ; Just return from interrupt