#include "interrupts.h"
#include "drivers.h"
#include "string.h"
#include "kernel.h"
#include "printk.h"

idt_entry_t idt[256];
idt_ptr_t idtp;
extern uint64_t gdt64;
struct tss_struct tss;

static char double_fault_stack[4096] __attribute__((aligned(16)));
static char page_fault_stack[4096] __attribute__((aligned(16)));
static char general_protection_stack[4096] __attribute__((aligned(16)));

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline void io_wait(void) {
    outb(0x80, 0);
}

void PIC_sendEOI(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

void PIC_remap(uint8_t offset1, uint8_t offset2) {
    // save masks
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);
    
    // start initialization sequence
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    
    // set vector offsets
    outb(PIC1_DATA, offset1);
    io_wait();
    outb(PIC2_DATA, offset2);
    io_wait();
    
    // set up master/slave relationship
    outb(PIC1_DATA, 4);
    io_wait();
    outb(PIC2_DATA, 2);
    io_wait();
    
    // set 8086 mode
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();
    
    // restore masks
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

void IRQ_init(void) {
    for (int i = 0; i < 16; i++) {
        irq_table[i].handler = NULL;
        irq_table[i].arg = NULL;
    }
    idt_init();
    PIC_remap(0x20, 0x28);
    IRQ_set_handler(1, kb_interrupt_handler, NULL);
    __asm__ volatile("sti");
}

// set IRQ mask (disable an IRQ)
void IRQ_set_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    value = inb(port) | (1 << irq);
    outb(port, value);
}

// clear IRQ mask (enable an IRQ)
void IRQ_clear_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    value = inb(port) & ~(1 << irq);
    outb(port, value);
}

void idt_init(void) {
    setup_tss();
    idtp.limit = (sizeof(idt_entry_t) * 256) - 1;
    idtp.base = (uint64_t)&idt;
    memset(&idt, 0, sizeof(idt_entry_t) * 256);
    
    // exception handlers (0-31)
    idt_set_gate(0, (uint64_t)isr_0, 0x08, 0, 0x8E);
    idt_set_gate(1, (uint64_t)isr_1, 0x08, 0, 0x8E);
    idt_set_gate(2, (uint64_t)isr_2, 0x08, 0, 0x8E);
    idt_set_gate(3, (uint64_t)isr_3, 0x08, 0, 0x8E);
    idt_set_gate(4, (uint64_t)isr_4, 0x08, 0, 0x8E);
    idt_set_gate(5, (uint64_t)isr_5, 0x08, 0, 0x8E);
    idt_set_gate(6, (uint64_t)isr_6, 0x08, 0, 0x8E);
    idt_set_gate(7, (uint64_t)isr_7, 0x08, 0, 0x8E);
    idt_set_gate(8, (uint64_t)isr_8, 0x08, 1, 0x8E);  // Double Fault uses IST1
    idt_set_gate(9, (uint64_t)isr_9, 0x08, 0, 0x8E);
    idt_set_gate(10, (uint64_t)isr_10, 0x08, 0, 0x8E);
    idt_set_gate(11, (uint64_t)isr_11, 0x08, 0, 0x8E);
    idt_set_gate(12, (uint64_t)isr_12, 0x08, 0, 0x8E);
    idt_set_gate(13, (uint64_t)isr_13, 0x08, 3, 0x8E); // GP Fault uses IST3
    idt_set_gate(14, (uint64_t)isr_14, 0x08, 2, 0x8E); // Page Fault uses IST2
    idt_set_gate(15, (uint64_t)isr_15, 0x08, 0, 0x8E);
    idt_set_gate(16, (uint64_t)isr_16, 0x08, 0, 0x8E);
    idt_set_gate(17, (uint64_t)isr_17, 0x08, 0, 0x8E);
    idt_set_gate(18, (uint64_t)isr_18, 0x08, 0, 0x8E);
    idt_set_gate(19, (uint64_t)isr_19, 0x08, 0, 0x8E);
    idt_set_gate(20, (uint64_t)isr_20, 0x08, 0, 0x8E);
    idt_set_gate(21, (uint64_t)isr_21, 0x08, 0, 0x8E);
    idt_set_gate(22, (uint64_t)isr_22, 0x08, 0, 0x8E);
    idt_set_gate(23, (uint64_t)isr_23, 0x08, 0, 0x8E);
    idt_set_gate(24, (uint64_t)isr_24, 0x08, 0, 0x8E);
    idt_set_gate(25, (uint64_t)isr_25, 0x08, 0, 0x8E);
    idt_set_gate(26, (uint64_t)isr_26, 0x08, 0, 0x8E);
    idt_set_gate(27, (uint64_t)isr_27, 0x08, 0, 0x8E);
    idt_set_gate(28, (uint64_t)isr_28, 0x08, 0, 0x8E);
    idt_set_gate(29, (uint64_t)isr_29, 0x08, 0, 0x8E);
    idt_set_gate(30, (uint64_t)isr_30, 0x08, 0, 0x8E);
    idt_set_gate(31, (uint64_t)isr_31, 0x08, 0, 0x8E);
    
    // IRQ handlers (32-47)
    idt_set_gate(32, (uint64_t)isr_32, 0x08, 0, 0x8E); // Timer
    idt_set_gate(33, (uint64_t)isr_33, 0x08, 0, 0x8E); // Keyboard
    idt_set_gate(34, (uint64_t)isr_34, 0x08, 0, 0x8E);
    idt_set_gate(35, (uint64_t)isr_35, 0x08, 0, 0x8E);
    idt_set_gate(36, (uint64_t)isr_36, 0x08, 0, 0x8E);
    idt_set_gate(37, (uint64_t)isr_37, 0x08, 0, 0x8E);
    idt_set_gate(38, (uint64_t)isr_38, 0x08, 0, 0x8E);
    idt_set_gate(39, (uint64_t)isr_39, 0x08, 0, 0x8E);
    idt_set_gate(40, (uint64_t)isr_40, 0x08, 0, 0x8E);
    idt_set_gate(41, (uint64_t)isr_41, 0x08, 0, 0x8E);
    idt_set_gate(42, (uint64_t)isr_42, 0x08, 0, 0x8E);
    idt_set_gate(43, (uint64_t)isr_43, 0x08, 0, 0x8E);
    idt_set_gate(44, (uint64_t)isr_44, 0x08, 0, 0x8E);
    idt_set_gate(45, (uint64_t)isr_45, 0x08, 0, 0x8E);
    idt_set_gate(46, (uint64_t)isr_46, 0x08, 0, 0x8E);
    idt_set_gate(47, (uint64_t)isr_47, 0x08, 0, 0x8E);

    for (int i = 48; i < 256; i++) {
        idt_set_gate(i, (uint64_t)default_interrupt, 0x08, 0, 0x8E);
    }

    idt_load();
}

void idt_set_gate(uint8_t num, uint64_t handler, uint16_t selector, uint8_t ist, uint8_t type_attr) {
    idt[num].isr_low = handler & 0xFFFF;
    idt[num].kernel_cs = selector;
    idt[num].ist = ist;
    idt[num].attributes = type_attr;
    idt[num].isr_mid = (handler >> 16) & 0xFFFF;
    idt[num].isr_high = (handler >> 32) & 0xFFFFFFFF;
    idt[num].reserved = 0;
}

void IRQ_set_handler(int irq, irq_handler_t handler, void* arg) {
    if (irq >= 0 && irq < 16) {
        irq_table[irq].handler = handler;
        irq_table[irq].arg = arg;
        printk("Handler set for IRQ %d\n", irq);
    } else {
        printk("Error: Invalid IRQ number %d\n", irq);
    }
}

void interrupt_handler(struct interrupt_frame* frame) {
    uint64_t fault_address;
    switch (frame->int_no) {
        // CPU exceptions
        case 0:
            printk("Division by Zero Exception\n");
            break;
        case 1:
            printk("Debug Exception\n");
            break;
        case 2:
            printk("Non-maskable Interrupt\n");
            break;
        case 3:
            printk("Breakpoint Exception\n");
            break;
        case 4:
            printk("Overflow Exception\n");
            break;
        case 5:
            printk("Bound Range Exceeded Exception\n");
            break;
        case 6:
            printk("Invalid Opcode Exception\n");
            break;
        case 7:
            printk("Device Not Available Exception\n");
            break;
        case 8:
            printk("Double Fault Exception\n");
            // double fault should never return
            while(1) {
                __asm__ volatile("cli");
                __asm__ volatile("hlt");
            }
            break;
        case 10:
            printk("Invalid TSS Exception\n");
            break;
        case 11:
            printk("Segment Not Present Exception\n");
            break;
        case 12:
            printk("Stack-Segment Fault Exception\n");
            break;
        case 13:
            printk("General Protection Fault\n");
            printk("Error code: %lx\n", frame->err_code);
            break;
        case 14:
            printk("Page Fault\n");
            printk("Error code: %lx\n", frame->err_code);
            // get the address that caused the fault
            __asm__ volatile("mov %%cr2, %0" : "=r" (fault_address));
            printk("Fault address: %lx\n", fault_address);
            break;
        
        // hardware interrupts
        case 32:
            // timer interrupt (IRQ0)
            if (irq_table[0].handler)
                irq_table[0].handler(0, frame->err_code, irq_table[0].arg);
            PIC_sendEOI(0);
            break;
        case 33:
            // keyboard interrupt (IRQ1)
            if (irq_table[1].handler) {
                irq_table[1].handler(1, frame->err_code, irq_table[1].arg);
            } else {
                inb(PS2_DATA);
                printk("Keyboard interrupt received but no handler registered\n");
            }
            PIC_sendEOI(1);
            break;
            
        // additional hardware IRQs (34-47)
        default:
            if (frame->int_no >= 32 && frame->int_no < 48) {
                int irq = frame->int_no - 32;
                if (irq_table[irq].handler)
                    irq_table[irq].handler(irq, frame->err_code, irq_table[irq].arg);
                PIC_sendEOI(irq);
            } else {
                printk("Unhandled Interrupt: %ld\n", frame->int_no);
            }
            break;
    }
}

void setup_tss(void) {
    memset(&tss, 0, sizeof(tss));
    
    // set up IST entries (stack grows down, so point to the end)
    tss.ist1 = (uint64_t)&double_fault_stack[sizeof(double_fault_stack)];
    tss.ist2 = (uint64_t)&page_fault_stack[sizeof(page_fault_stack)];
    tss.ist3 = (uint64_t)&general_protection_stack[sizeof(general_protection_stack)];
    
    // set I/O permission bitmap offset to the size of the TSS
    tss.iopb_offset = sizeof(tss);
    
    // get base address and limit of TSS
    uint64_t tss_base = (uint64_t)&tss;
    uint32_t tss_limit = sizeof(tss) - 1;
    
    // tss descriptor
    struct tss_gdt_entry tss_entry = {0};
    tss_entry.limit_low = tss_limit & 0xFFFF;
    tss_entry.base_low = tss_base & 0xFFFF;
    tss_entry.base_middle = (tss_base >> 16) & 0xFF;
    tss_entry.access = 0x89;  // Present, DPL=0, Type=9 (64-bit TSS)
    tss_entry.granularity = ((tss_limit >> 16) & 0x0F);
    tss_entry.base_high = (tss_base >> 24) & 0xFF;
    tss_entry.base_upper = (tss_base >> 32) & 0xFFFFFFFF;
    tss_entry.reserved = 0;

    // 4th entry is tss descriptor
    memcpy((void*)(&gdt64 + 3), &tss_entry, sizeof(tss_entry));
    __asm__ volatile("ltr %%ax" : : "a"(0x18));
}