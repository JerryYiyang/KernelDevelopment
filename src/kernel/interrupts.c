#include "interrupts.h"
#include "drivers.h"
#include "string.h"

idt_entry_t idt[256];
idt_ptr_t idtp;

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
    
    // start init sequence
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
    idt_init();
    PIC_remap(0x20, 0x28);
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
    idtp.limit = (sizeof(idt_entry_t) * 256) - 1;
    idtp.base = (uint64_t)&idt;
    memset(&idt, 0, sizeof(idt_entry_t) * 256);
    
    // set all IDT entries to use testirq handler for initial testing
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, (uint64_t)testirq, 0x08, 0, 0x8E);
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
    irq_table[irq].handler = handler;
    irq_table[irq].arg = arg;
}

void interrupt_handler(void) {
    // do nothing for now
}

__attribute__((noreturn)) void exception_handler(void) {
    // handle exception
    for(;;); // halt on exception
}