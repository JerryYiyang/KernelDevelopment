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

struct tss_struct {
    uint32_t reserved1;
    uint64_t rsp0;        // Ring 0 stack pointer
    uint64_t rsp1;        // Ring 1 stack pointer
    uint64_t rsp2;        // Ring 2 stack pointer
    uint64_t reserved2;
    uint64_t ist1;        // IST1 - Double fault
    uint64_t ist2;        // IST2 - Page fault
    uint64_t ist3;        // IST3 - General protection fault
    uint64_t ist4;        // IST4
    uint64_t ist5;        // IST5
    uint64_t ist6;        // IST6
    uint64_t ist7;        // IST7
    uint64_t reserved3;
    uint16_t reserved4;
    uint16_t iopb_offset; // I/O map base address
} __attribute__((packed));

struct tss_gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle; // where tss starts
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
    uint32_t base_upper;
    uint32_t reserved;
} __attribute__((packed));

void setup_tss(void);

#endif