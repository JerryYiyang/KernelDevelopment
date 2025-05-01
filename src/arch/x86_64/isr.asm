; isr.asm - Interrupt Service Routines for x86_64
section .text
bits 64

; External C functions
extern interrupt_handler

; Export all symbols
global idt_load
global default_interrupt
global isr_0
global isr_1
global isr_2
global isr_3
global isr_4
global isr_5
global isr_6
global isr_7
global isr_8
global isr_9
global isr_10
global isr_11
global isr_12
global isr_13
global isr_14
global isr_15
global isr_16
global isr_17
global isr_18
global isr_19
global isr_20
global isr_21
global isr_22
global isr_23
global isr_24
global isr_25
global isr_26
global isr_27
global isr_28
global isr_29
global isr_30
global isr_31
global isr_32
global isr_33
global isr_34
global isr_35
global isr_36
global isr_37
global isr_38
global isr_39
global isr_40
global isr_41
global isr_42
global isr_43
global isr_44
global isr_45
global isr_46
global isr_47

; Load IDT
extern idtp
idt_load:
    lidt [idtp]
    ret

align 16
default_interrupt:
    push qword 0       ; push dummy error code
    push qword 0xFF    ; push a special number to identify default handler
    jmp isr_common     ; jump to common handler

; ISRs for exceptions that don't push an error code
%macro ISR_NO_ERR 1
align 16
isr_%1:
    push qword 0       ; push dummy error code
    push qword %1      ; push interrupt number
    jmp isr_common     ; jump to common handler
%endmacro

; ISRs for exceptions that do push an error code
%macro ISR_ERR 1
align 16
isr_%1:
    push qword %1      ; push interrupt number (error code already on stack)
    jmp isr_common
%endmacro

; define CPU exception handlers (0-31)
ISR_NO_ERR 0   ; Divide by Zero
ISR_NO_ERR 1   ; Debug
ISR_NO_ERR 2   ; Non-maskable Interrupt
ISR_NO_ERR 3   ; Breakpoint
ISR_NO_ERR 4   ; Overflow
ISR_NO_ERR 5   ; Bound Range Exceeded
ISR_NO_ERR 6   ; Invalid Opcode
ISR_NO_ERR 7   ; Device Not Available
ISR_ERR    8   ; Double Fault (uses IST1)
ISR_NO_ERR 9   ; Coprocessor Segment Overrun
ISR_ERR    10  ; Invalid TSS
ISR_ERR    11  ; Segment Not Present
ISR_ERR    12  ; Stack-Segment Fault
ISR_ERR    13  ; General Protection Fault (uses IST3)
ISR_ERR    14  ; Page Fault (uses IST2)
ISR_NO_ERR 15  ; Reserved
ISR_NO_ERR 16  ; x87 FPU Error
ISR_ERR    17  ; Alignment Check
ISR_NO_ERR 18  ; Machine Check
ISR_NO_ERR 19  ; SIMD Floating-point Exception
ISR_NO_ERR 20  ; Virtualization Exception
ISR_ERR    21  ; Control Protection Exception
ISR_NO_ERR 22  ; Reserved
ISR_NO_ERR 23  ; Reserved
ISR_NO_ERR 24  ; Reserved
ISR_NO_ERR 25  ; Reserved
ISR_NO_ERR 26  ; Reserved
ISR_NO_ERR 27  ; Reserved
ISR_NO_ERR 28  ; Reserved
ISR_NO_ERR 29  ; Reserved
ISR_ERR    30  ; Reserved
ISR_NO_ERR 31  ; Reserved

; define IRQs (32-47)
ISR_NO_ERR 32  ; Timer (IRQ0)
ISR_NO_ERR 33  ; Keyboard (IRQ1)
ISR_NO_ERR 34  ; Cascade for Slave PIC (IRQ2)
ISR_NO_ERR 35  ; COM2 (IRQ3)
ISR_NO_ERR 36  ; COM1 (IRQ4)
ISR_NO_ERR 37  ; LPT2 (IRQ5)
ISR_NO_ERR 38  ; Floppy Disk (IRQ6)
ISR_NO_ERR 39  ; LPT1 / Spurious (IRQ7)
ISR_NO_ERR 40  ; CMOS Real Time Clock (IRQ8)
ISR_NO_ERR 41  ; Free / SCSI / NIC (IRQ9)
ISR_NO_ERR 42  ; Free / SCSI / NIC (IRQ10)
ISR_NO_ERR 43  ; Free / SCSI / NIC (IRQ11)
ISR_NO_ERR 44  ; PS2 Mouse (IRQ12)
ISR_NO_ERR 45  ; FPU / Coprocessor / Inter-processor (IRQ13)
ISR_NO_ERR 46  ; Primary ATA Hard Disk (IRQ14)
ISR_NO_ERR 47  ; Secondary ATA Hard Disk (IRQ15)

align 16
isr_common:
    ; save all registers
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; save original data segment
    mov ax, ds
    push rax
    
    ; load kernel data segment
    mov ax, 0x10       ; kernel data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; call C handler (passing pointer to stack as argument)
    mov rdi, rsp
    call interrupt_handler
    
    ; restore original data segment
    pop rax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; restore all registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    
    ; clean up error code and interrupt number
    add rsp, 16
    iretq