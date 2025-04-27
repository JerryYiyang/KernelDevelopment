#include "interrupts.h"
#include "string.h"

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0"
                    : "=a"(ret)
                    : "Nd"(port) );
    return ret;
}

// disable interrupts
static inline void cli(void) {
    asm volatile("cli");
}

// enable interrupts
static inline void sti(void) {
    asm volatile("sti");
}

// Small delay for PIC to settle
static inline void io_wait(void) {
    outb(0x80, 0);
}

/*-------------------PIC-------------------*/

void PIC_sendEOI(uint8_t irq) {
	if(irq >= 8)
		outb(PIC2_COMMAND,PIC_EOI);
	
	outb(PIC1_COMMAND,PIC_EOI);
}

/*
arguments:
	offset1 - vector offset for master PIC
		vectors on the master become offset1..offset1+7
	offset2 - same for slave PIC: offset2..offset2+7
*/
static void PIC_remap(uint8_t offset1, uint8_t offset2) {
    // Save masks
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);

    // Start initialization sequence (in cascade mode)
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    
    // ICW2: Set new offsets (interrupt vector numbers)
    outb(PIC1_DATA, offset1);     // Master PIC starts at offset1
    io_wait();
    outb(PIC2_DATA, offset2);     // Slave PIC starts at offset2
    io_wait();
    
    // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    outb(PIC1_DATA, 4);          // Slave connected to IRQ2 (0000 0100)
    io_wait();
    // ICW3: tell Slave PIC its cascade identity (0000 0010)
    outb(PIC2_DATA, 2);          // Slave's identity is 2
    io_wait();
    
    // ICW4: have the PICs use 8086 mode (and not 8080 mode)
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();
    
    // Unmask both PICs.
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

/*-------------------Interrupts-------------------*/

// idt structs
__attribute__((aligned(0x10))) 
static struct idt_entry_t idt[256];
static struct idtr_t idtr;

// irq handler table
static irq_handler_t irq_handlers[16] = {0};
static void* irq_args[16] = {0};

void interrupts_init(void) {
    idt_init();
    IRQ_init();
}

void IRQ_init(void) {
    cli();
    PIC_remap(0x20, 0x28);
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;
    memset(&idt, 0, sizeof(struct idt_entry) * 256);
    sti();
}

void IRQ_set_mask(uint8_t IRQline) {
    uint16_t port;
    uint8_t value;

    if(IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) | (1 << IRQline);
    outb(port, value);        
}

void IRQ_clear_mask(uint8_t IRQline) {
    uint16_t port;
    uint8_t value;

    if(IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) & ~(1 << IRQline);
    outb(port, value);        
}

void idt_init(void) {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint64_t)&idt;
    
    // init all IDT entries to zero
    for (int i = 0; i < 256; i++) {
        idt[i].target_offset_low = 0;
        idt[i].target_selector = 0;
        idt[i].ist = 0;
        idt[i].type_attr = 0;
        idt[i].target_offset_middle = 0;
        idt[i].target_offset_high = 0;
        idt[i].reserved = 0;
    }
    
    // Set up CPU exception handlers


    idt_load();
}

void idt_set_gate(uint8_t num, uint64_t handler, uint16_t selector, uint8_t ist, uint8_t type_attr) {
    idt[num].target_offset_low = handler & 0xFFFF;
    idt[num].target_selector = selector;
    idt[num].ist = ist;
    idt[num].type_attr = type_attr;
    idt[num].target_offset_middle = (handler >> 16) & 0xFFFF;
    idt[num].target_offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[num].reserved = 0;
}

void exception_handler() {
    __asm__ volatile ("cli; hlt"); // Completely hangs the computer
}

void interrupt_handler(void) {

}