#include "interrupts.h"

// disable interrupts
static inline void cli(void) {
    asm volatile("cli");
}

// enable interrupts
static inline void sti(void) {
    asm volatile("sti");
}

void IRQ_init(void) {
    cli();
    
}