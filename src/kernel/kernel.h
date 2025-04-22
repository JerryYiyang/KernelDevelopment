#ifndef KERNEL_H
#define KERNEL_H

static inline void lgdt(void* base, uint16_t size) {
    static struct {
        uint16_t length;
        void* base;
    } __attribute__((packed)) GDTR;

    GDTR.length = size;
    GDTR.base = base;

    asm ( "lgdt %0" : : "m"(GDTR) );
}

#endif