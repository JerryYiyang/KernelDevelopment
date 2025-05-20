#ifndef MMU_H
#define MMU_H

#include "interrupts.h"
#include <stdint.h>
#include <stddef.h>

#define MULTIBOOT2_INFO_MAGIC 0x36d76289    // val in EAX when kernel is loaded
#define MULTIBOOT2_HEADER_MAGIC 0xE85250D6  // val at the start of kernel binary header
#define MULTIBOOT2_HEADER_ALIGN 8           // 8-byte alignment required

#define MULTIBOOT2_ARCHITECTURE_X86 0       // 32-bit (protected) mode
#define MULTIBOOT2_ARCHITECTURE_X64 4       // 64-bit (long) mode

#define MULTIBOOT_TAG_TYPE_END               0
#define MULTIBOOT_TAG_TYPE_CMDLINE           1
#define MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME  2
#define MULTIBOOT_TAG_TYPE_MODULE            3
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO     4
#define MULTIBOOT_TAG_TYPE_BOOTDEV           5
#define MULTIBOOT_TAG_TYPE_MMAP              6
#define MULTIBOOT_TAG_TYPE_VBE               7
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER       8
#define MULTIBOOT_TAG_TYPE_ELF_SECTIONS      9
#define MULTIBOOT_TAG_TYPE_APM               10
#define MULTIBOOT_TAG_TYPE_EFI32             11
#define MULTIBOOT_TAG_TYPE_EFI64             12
#define MULTIBOOT_TAG_TYPE_SMBIOS            13
#define MULTIBOOT_TAG_TYPE_ACPI_OLD          14
#define MULTIBOOT_TAG_TYPE_ACPI_NEW          15
#define MULTIBOOT_TAG_TYPE_NETWORK           16
#define MULTIBOOT_TAG_TYPE_EFI_MMAP          17

#define MULTIBOOT_MEMORY_AVAILABLE           1
#define MULTIBOOT_MEMORY_RESERVED            2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE    3
#define MULTIBOOT_MEMORY_NVS                 4
#define MULTIBOOT_MEMORY_BADRAM              5

// section types
#define SHT_NULL        0          // inactive section header
#define SHT_PROGBITS    1          // program data
#define SHT_SYMTAB      2          // symbol table
#define SHT_STRTAB      3          // string table
#define SHT_RELA        4          // relocation with addends
#define SHT_HASH        5          // symbol hash table
#define SHT_DYNAMIC     6          // dynamic linking information
#define SHT_NOTE        7          // note information
#define SHT_NOBITS      8          // uninitialized data (like .bss)
#define SHT_REL         9          // relocation entries without addends
#define SHT_SHLIB       10         // reserved
#define SHT_DYNSYM      11         // dynamic symbol table

// section flags
#define SHF_WRITE       0x1        // contains writable data
#define SHF_ALLOC       0x2        // occupies memory
#define SHF_EXECINSTR   0x4        // contains executable code
#define SHF_MERGE       0x10       // can be merged
#define SHF_STRINGS     0x20       // contains strings
#define SHF_INFO_LINK   0x40       // section's sh_info holds section index
#define SHF_LINK_ORDER  0x80       // special link ordering requirements
#define SHF_OS_NONCONFORMING 0x100 // OS-specific handling required
#define SHF_GROUP       0x200      // member of a group
#define SHF_TLS         0x400      // contains TLS data

// 4kb
#define PAGE_SIZE 4096

#define MAX_MEMORY_REGIONS 64

struct multiboot2_header {
    uint32_t total_size;
    uint32_t reserved;    // must be zero
};

struct multiboot2_tag {
    uint32_t type;
    uint32_t size;
};

struct multiboot2_tag_basic_meminfo {
    uint32_t type;
    uint32_t size;
    uint32_t mem_lower;
    uint32_t mem_upper;
};

struct multiboot2_tag_string {
    uint32_t type;
    uint32_t size;
    char string[0];
};

struct multiboot2_mmap_entry {
    uint64_t addr;  // base address
    uint64_t len;
    uint32_t type;
    uint32_t zero;  // reserved, must be zero
};

struct multiboot2_tag_mmap {
    uint32_t type;
    uint32_t size; // 24 bytes
    uint32_t entry_size;
    uint32_t entry_version;
    struct multiboot2_mmap_entry entries[0];
};

struct multiboot2_tag_elf_sections {
    uint32_t type;
    uint32_t size;
    uint32_t num;      // num section headers
    uint32_t entsize;  // size of one section header entry
    uint32_t shndx;    // index of section containing section name strings
    char sections[0];  // array of section headers
};

struct elf64_shdr {
    uint32_t sh_name;      // index into string table
    uint32_t sh_type;      // section type
    uint64_t sh_flags;     // section attributes
    uint64_t sh_addr;      // virtual address in memory
    uint64_t sh_offset;    // offset in file
    uint64_t sh_size;      // size of section
    uint32_t sh_link;      // link to another section
    uint32_t sh_info;      // additional information
    uint64_t sh_addralign; // address alignment boundary
    uint64_t sh_entsize;   // entry size if section holds a table
};

struct mem_region {
    uint64_t start;   // start address (inclusive)
    uint64_t end;     // end address (exclusive)
    uint32_t type;    // region type (1 = available, 2 = reserved, etc.)
};

struct free_page {
    struct free_page *next;
};

void MMU_init(uint64_t multiboot_info);
void *MMU_pf_alloc(void);
void MMU_pf_free(void *pf);
void MMU_print_memory_map(void);

// virtual address space
// go into boot.asm and change the page table there to match this struct (make sure to update cr3)
// after that, it should be safe to use cr3, and for multiprocessing, pass in the cr3 as a param for them (later on)
// write funcs to walk through the page table
#define PHYS_PF_ADR 0x0000000000                // PML4E slot 0
#define KERNEL_HEAP_ADR 0x10000000000           // PML4E slot 1
#define KERNEL_GROWTH_ADR_START 0x20000000000   // PML4E slot 2 - 14
#define KERNEL_GROWTH_ADR_END 0xEFFFFFFFFFF
#define KERNEL_STACKS_ADR 0xF0000000000         // PML4E slot 15
#define USER_SPACE_ADR 0x100000000000           // PML4E slot 16

#define PTE_PRESENT (1ULL << 0) // ULL to make sure its a 64 bit int
#define PTE_WRITABLE (1ULL << 1)
#define PTE_USER (1ULL << 2)
#define PTE_WRITETHROUGH (1ULL << 3)
#define PTE_NOT_CACHEABLE (1ULL << 4)
#define PTE_ACCESSED (1ULL << 5)
#define PTE_DIRTY (1ULL << 6)
#define PTE_HUGE (1ULL << 7)
#define PTE_GLOBAL (1ULL << 8)
#define PTE_DEMAND_PAGING (1ULL << 9) 
#define PTE_NX (1ULL << 63)

#define PAGE_SHIFT 12
#define ENTRY_PER_TABLE 512
#define PML4_SHIFT 39
#define PDPT_SHIFT 30
#define PD_SHIFT 21
#define PT_SHIFT 12
#define PAGE_MASK (~(PAGE_SIZE - 1))
#define PML4E_BITS 9

void set_cr3(uint64_t cr3_value);
void invlpg(void *addr);
uint64_t virt_to_phys(void *vaddr);
uint64_t* get_pte(uint64_t *pml4t, uint64_t vaddr, int create_if_not_exist);
void map_page(uint64_t *pml4t, uint64_t vaddr, uint64_t paddr, uint64_t flags);
void unmap_page(uint64_t *pml4t, uint64_t vaddr);
void page_fault_handler(struct interrupt_frame* frame);
void* MMU_alloc_page(void);
void* MMU_alloc_pages(int num);
void MMU_free_page(void *vaddr);
void MMU_free_pages(void *vaddr, int num);

#endif