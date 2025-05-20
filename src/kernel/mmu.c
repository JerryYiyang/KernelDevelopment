#include "mmu.h"
#include "printk.h"
#include "string.h"

static struct mem_region memory_regions[MAX_MEMORY_REGIONS];
static int num_memory_regions = 0;
static struct free_page *free_page_head = NULL;
static uint64_t total_memory = 0;
static uint64_t total_pages = 0;
static uint64_t free_pages = 0;
static uint64_t reserved_pages = 0;
static int free_list_initialized = 0;

static void process_mmap_tag(struct multiboot2_tag_mmap *mmap_tag) {
    uint8_t *entry_ptr = (uint8_t *)mmap_tag->entries;
    uint8_t *end_ptr = (uint8_t *)mmap_tag + mmap_tag->size;
    while (entry_ptr < end_ptr) {
        struct multiboot2_mmap_entry *entry = (struct multiboot2_mmap_entry *)entry_ptr;
        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            if (num_memory_regions < MAX_MEMORY_REGIONS) {
                // aligning to page boundaries
                uint64_t start = (entry->addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
                uint64_t end = (entry->addr + entry->len) & ~(PAGE_SIZE - 1);
                if (start < end) {
                    memory_regions[num_memory_regions].start = start;
                    memory_regions[num_memory_regions].end = end;
                    memory_regions[num_memory_regions].type = entry->type;
                    num_memory_regions++;
                    uint64_t region_size = end - start;
                    total_memory += region_size;
                    uint64_t region_pages = region_size / PAGE_SIZE;
                    total_pages += region_pages;
                    free_pages += region_pages;
                }
            } else {
                printk("WARNING: Too many memory regions, ignoring some\n");
            }
        }
        entry_ptr += mmap_tag->entry_size;
    }
}

static void process_elf_sections_tag(struct multiboot2_tag_elf_sections *elf_tag) {
    for (uint32_t i = 0; i < elf_tag->num; i++) {
        struct elf64_shdr *shdr = (struct elf64_shdr *)(elf_tag->sections + i * elf_tag->entsize);
        // skip sections that don't occupy memory
        if (!(shdr->sh_flags & SHF_ALLOC)) {
            continue;
        }
        // determine section type (code, data, bss, etc.)
        const char *section_type = "idk";
        if (shdr->sh_type == SHT_PROGBITS) {
            if (shdr->sh_flags & SHF_EXECINSTR) {
                section_type = "code";
            } else if (shdr->sh_flags & SHF_WRITE) {
                section_type = "data";
            } else {
                section_type = "read-only";
            }
        } else if (shdr->sh_type == SHT_NOBITS) {
            section_type = "bss";
        }
        uint64_t start = shdr->sh_addr;
        uint64_t end = shdr->sh_addr + shdr->sh_size;
        // skip empty sections
        if (start == end) {
            continue;
        }
        printk("  Kernel section (%s): addr=0x%lx, size=0x%lx\n", section_type, start, shdr->sh_size);
        // aligning to page size
        uint64_t page_start = start & ~(PAGE_SIZE - 1);
        uint64_t page_end = (end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        // mark pages used as reserved
        for (int j = 0; j < num_memory_regions; j++) {
            // skip non-available memory regions
            if (memory_regions[j].type != MULTIBOOT_MEMORY_AVAILABLE) {
                continue;
            }
            if (!(page_start >= memory_regions[j].end || page_end <= memory_regions[j].start)) {
                uint64_t overlap_start = (page_start > memory_regions[j].start) ? 
                                       page_start : memory_regions[j].start;
                uint64_t overlap_end = (page_end < memory_regions[j].end) ? 
                                     page_end : memory_regions[j].end;
                // skip if there's no actual overlap after alignment
                if (overlap_start >= overlap_end) {
                    continue;
                }
                // updating stats for reserved pages
                uint64_t overlap_pages = (overlap_end - overlap_start) / PAGE_SIZE;
                free_pages -= overlap_pages;
                reserved_pages += overlap_pages;
                // dealing with overalp
                if (overlap_start == memory_regions[j].start && 
                    overlap_end == memory_regions[j].end) {
                    // completely overlapped - mark as reserved
                    memory_regions[j].type = MULTIBOOT_MEMORY_RESERVED;
                } 
                else if (overlap_start == memory_regions[j].start) {
                    // overlaps beginning - adjust start address
                    memory_regions[j].start = overlap_end;
                } 
                else if (overlap_end == memory_regions[j].end) {
                    // overlaps end - adjust end address
                    memory_regions[j].end = overlap_start;
                } 
                else if (num_memory_regions < MAX_MEMORY_REGIONS) {
                    // splits region in two - create new region
                    memory_regions[num_memory_regions].start = overlap_end;
                    memory_regions[num_memory_regions].end = memory_regions[j].end;
                    memory_regions[num_memory_regions].type = MULTIBOOT_MEMORY_AVAILABLE;
                    num_memory_regions++;
                    memory_regions[j].end = overlap_start;
                } 
                else {
                    printk("WARNING: No space for split region, marking entire region as reserved\n");
                    memory_regions[j].type = MULTIBOOT_MEMORY_RESERVED;
                }
            }
        }
    }
}

void MMU_init(uint64_t multiboot_info) {
    struct multiboot2_header *mbi = (struct multiboot2_header *)multiboot_info;
    // process all tags after 8 byte header
    uint8_t *current = (uint8_t *)multiboot_info + 8;
    uint8_t *end = (uint8_t *)multiboot_info + mbi->total_size;
    while (current < end) {
        struct multiboot2_tag *tag = (struct multiboot2_tag *)current;
        if (tag->type == MULTIBOOT_TAG_TYPE_END) {
            break;
        }
        if (tag->size < 8 || current + tag->size > end) {
            printk("ERROR: Invalid tag size at 0x%lx\n", (uint64_t)current);
            return;
        }
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_MMAP:
                process_mmap_tag((struct multiboot2_tag_mmap *)tag);
                break;
            case MULTIBOOT_TAG_TYPE_ELF_SECTIONS:
                process_elf_sections_tag((struct multiboot2_tag_elf_sections *)tag);
                break;                
            case MULTIBOOT_TAG_TYPE_CMDLINE:
                printk("Command line: %s\n", 
                       ((struct multiboot2_tag_string *)tag)->string);
                break;               
            case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
                printk("Boot loader: %s\n", 
                       ((struct multiboot2_tag_string *)tag)->string);
                break;               
            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
                printk("Basic memory info: lower=%uKB, upper=%uKB\n",
                       ((struct multiboot2_tag_basic_meminfo *)tag)->mem_lower,
                       ((struct multiboot2_tag_basic_meminfo *)tag)->mem_upper);
                break;
            default:
                printk("Skipping tag type %u of size %u\n", tag->type, tag->size);
                break;
        }
        current = (uint8_t *)tag + ((tag->size + 7) & ~7);
    }
    printk("Memory management initialized:\n");
    printk("  Total memory: %lu MB\n", total_memory / (1024 * 1024));
    printk("  Total pages: %lu\n", total_pages);
    printk("  Free pages: %lu\n", free_pages);
    printk("  Reserved pages: %lu\n", reserved_pages);
    printk("  Memory regions: %d\n", num_memory_regions);
}

static void add_page_to_free_list(void *page) {
    struct free_page *fp = (struct free_page *)page;
    fp->next = free_page_head;
    free_page_head = fp;
}

static void init_free_page_list(void) {
    if (free_list_initialized) return;
    free_list_initialized = 1;
    for (int i = 0; i < num_memory_regions; i++) {
        if (memory_regions[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
            uint64_t start = memory_regions[i].start;
            uint64_t end = memory_regions[i].end;
            if (start < 0x100000) start = 0x100000;
            if (start >= end) continue;
            for (uint64_t addr = start; addr < end; addr += PAGE_SIZE) {
                add_page_to_free_list((void *)addr);
            }
        }
    }
}

void *MMU_pf_alloc(void) {
    if (!free_list_initialized) {
        init_free_page_list();
    }
    if (free_page_head == NULL) {
        printk("ERROR: Out of memory, no free pages available\n");
        return NULL;
    }
    struct free_page *page = free_page_head;
    free_page_head = page->next;
    memset(page, 0, PAGE_SIZE);
    free_pages--;
    return page;
}

void MMU_pf_free(void *pf) {
    if (pf == NULL) {
        printk("ERROR: Null page\n");
        return;
    }
    if ((uint64_t)pf % PAGE_SIZE != 0) {
        printk("ERROR: Trying to free unaligned page: 0x%p\n", pf);
        return;
    }
    add_page_to_free_list(pf);
    free_pages++;
}

void MMU_print_memory_map(void) {
    printk("\n======== Memory Map ========\n");
    printk("  Total memory: %lu MB\n", total_memory / (1024 * 1024));
    printk("  Total pages: %lu\n", total_pages);
    printk("  Free pages: %lu\n", free_pages);
    printk("  Reserved pages: %lu\n", reserved_pages);
    printk("\nMemory Regions (%d):\n", num_memory_regions);
    for (int i = 0; i < num_memory_regions; i++) {
        const char *type_str = "idk";
        switch (memory_regions[i].type) {
            case MULTIBOOT_MEMORY_AVAILABLE: type_str = "Available"; break;
            case MULTIBOOT_MEMORY_RESERVED: type_str = "Reserved"; break;
            case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE: type_str = "ACPI Reclaimable"; break;
            case MULTIBOOT_MEMORY_NVS: type_str = "NVS"; break;
            case MULTIBOOT_MEMORY_BADRAM: type_str = "Bad RAM"; break;
        }
        printk("  Region %d: 0x%lx - 0x%lx, Size: %lu MB, Type: %s\n", 
               i, memory_regions[i].start, memory_regions[i].end, 
               (memory_regions[i].end - memory_regions[i].start) / (1024 * 1024),
               type_str);
    }
    printk("============================\n\n");
}

// virtual addressing

static uint64_t kernel_brk = KERNEL_HEAP_ADR;

static inline uint64_t get_cr3(void) {
    uint64_t cr3_value;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3_value));
    return cr3_value;
}

// switch address space (multiple processes)
void set_cr3(uint64_t cr3_value) {
    __asm__ volatile("mov %0, %%cr3" : : "r"(cr3_value) : "memory");
}

// flush specified TLB entry
void invlpg(void *addr) {
    __asm__ volatile("invlpg (%0)" : : "r"(addr) : "memory");
}

static void get_page_indices(uint64_t vaddr, uint64_t *pml4_idx, uint64_t *pdpt_idx, 
                             uint64_t *pd_idx, uint64_t *pt_idx, uint64_t *offset) {
    *pml4_idx = (vaddr >> PML4_SHIFT) & (ENTRY_PER_TABLE - 1);
    *pdpt_idx = (vaddr >> PDPT_SHIFT) & (ENTRY_PER_TABLE - 1);
    *pd_idx = (vaddr >> PD_SHIFT) & (ENTRY_PER_TABLE - 1);
    *pt_idx = (vaddr >> PT_SHIFT) & (ENTRY_PER_TABLE - 1);
    *offset = vaddr & (PAGE_SIZE - 1);
}

static void* phys_to_virt(uint64_t paddr) {
    // its identity mapped so not much to do
    return (void*)paddr;
}

uint64_t virt_to_phys(void *vaddr) {
    uint64_t *pml4t, *pdpt, *pdt, *pt;
    uint64_t pml4_idx, pdpt_idx, pd_idx, pt_idx, offset;
    uint64_t pml4e, pdpte, pde, pte;

    pml4t = phys_to_virt(get_cr3() & PAGE_MASK);
    get_page_indices((uint64_t)vaddr, &pml4_idx, &pdpt_idx, &pd_idx, &pt_idx, &offset);

    pml4e = pml4t[pml4_idx];
    if (!(pml4e & PTE_PRESENT)) {
        printk("PML4 entry not present for address %p\n", vaddr);
        return 0;
    }
    pdpt = phys_to_virt(pml4e & PAGE_MASK);
    pdpte = pdpt[pdpt_idx];
    if (!(pdpte & PTE_PRESENT)) {
        printk("PDPT entry not present for address %p\n", vaddr);
        return 0;
    }
    
    // check if a 1GB page
    if (pdpte & PTE_HUGE) {
        return (pdpte & 0xFFFFFFC0000000ULL) + ((uint64_t)vaddr & 0x3FFFFFFF);
    }
    
    pdt = phys_to_virt(pdpte & PAGE_MASK);
    pde = pdt[pd_idx];
    if (!(pde & PTE_PRESENT)) {
        printk("PD entry not present for address %p\n", vaddr);
        return 0;
    }
    
    // check if a 2MB page
    if (pde & PTE_HUGE) {
        return (pde & 0xFFFFFFFE00000ULL) + ((uint64_t)vaddr & 0x1FFFFF);
    }
    
    pt = phys_to_virt(pde & PAGE_MASK);
    pte = pt[pt_idx];
    if (!(pte & PTE_PRESENT)) {
        printk("PT entry not present for address %p\n", vaddr);
        return 0;
    }
    
    return (pte & PAGE_MASK) + offset;
}

// gets addr of page table entry for virt addr
// can be used to modify page table entries
// if create_if_not_exist true, allocates missing page tables
uint64_t* get_pte(uint64_t *pml4t, uint64_t vaddr, int create_if_not_exist) {
    uint64_t *pdpt, *pdt, *pt;
    uint64_t pml4_idx, pdpt_idx, pd_idx, pt_idx, offset;
    uint64_t *pml4e, *pdpte, *pde;

    get_page_indices(vaddr, &pml4_idx, &pdpt_idx, &pd_idx, &pt_idx, &offset);
    pml4e = &pml4t[pml4_idx];
    if (!(*pml4e & PTE_PRESENT)) {
        if (!create_if_not_exist) {
            return NULL;
        }
        void *new_pdpt = MMU_pf_alloc();
        if (!new_pdpt) {
            printk("Failed to allocate PDPT\n");
            return NULL;
        }
        memset(new_pdpt, 0, PAGE_SIZE);
        *pml4e = ((uint64_t)new_pdpt & PAGE_MASK) | PTE_PRESENT | PTE_WRITABLE;
    }
    
    pdpt = phys_to_virt(*pml4e & PAGE_MASK);
    pdpte = &pdpt[pdpt_idx];
    if (!(*pdpte & PTE_PRESENT)) {
        if (!create_if_not_exist) {
            return NULL;
        }
        void *new_pd = MMU_pf_alloc();
        if (!new_pd) {
            printk("Failed to allocate PD\n");
            return NULL;
        }
        memset(new_pd, 0, PAGE_SIZE);
        *pdpte = ((uint64_t)new_pd & PAGE_MASK) | PTE_PRESENT | PTE_WRITABLE;
    }
    
    pdt = phys_to_virt(*pdpte & PAGE_MASK);
    pde = &pdt[pd_idx];
    if (!(*pde & PTE_PRESENT)) {
        if (!create_if_not_exist) {
            return NULL;
        }
        void *new_pt = MMU_pf_alloc();
        if (!new_pt) {
            printk("Failed to allocate PT\n");
            return NULL;
        }
        memset(new_pt, 0, PAGE_SIZE);
        *pde = ((uint64_t)new_pt & PAGE_MASK) | PTE_PRESENT | PTE_WRITABLE;
    }
    
    pt = phys_to_virt(*pde & PAGE_MASK);
    return &pt[pt_idx];
}

// map phys page to virt addr
void map_page(uint64_t *pml4t, uint64_t vaddr, uint64_t paddr, uint64_t flags) {
    uint64_t *pte = get_pte(pml4t, vaddr, 1);
    if (!pte) {
        printk("Failed to get PTE for address %lx\n", vaddr);
        return;
    }
    
    *pte = (paddr & PAGE_MASK) | flags | PTE_PRESENT;
    invlpg((void*)vaddr);
}

void unmap_page(uint64_t *pml4t, uint64_t vaddr) {
    uint64_t *pte = get_pte(pml4t, vaddr, 0);
    if (pte && (*pte & PTE_PRESENT)) {
        *pte = 0;
        invlpg((void*)vaddr);
    }
}

// demand paging
void* MMU_alloc_page(void) {
    // only allocates virt addr first
    void *vaddr = (void*)kernel_brk;
    kernel_brk += PAGE_SIZE;
    
    // maps page with demand paging flag set, but not present (causes page fault)
    uint64_t *pml4t = phys_to_virt(get_cr3() & PAGE_MASK);
    uint64_t *pte = get_pte(pml4t, (uint64_t)vaddr, 1);
    if (!pte) {
        printk("Failed to get PTE for address %lx\n", (uint64_t)vaddr);
        return NULL;
    }
    *pte = PTE_DEMAND_PAGING | PTE_WRITABLE;
    return vaddr;
}

void* MMU_alloc_pages(int num) {
    if (num <= 0) return NULL;
    void *start_addr = (void*)kernel_brk;
    uint64_t *pml4t = phys_to_virt(get_cr3() & PAGE_MASK);
    for (int i = 0; i < num; i++) {
        uint64_t vaddr = kernel_brk;
        uint64_t *pte = get_pte(pml4t, vaddr, 1);
        if (!pte) {
            // failed to set up page table
            MMU_free_pages(start_addr, i);
            return NULL;
        }
        *pte = PTE_DEMAND_PAGING | PTE_WRITABLE;
        kernel_brk += PAGE_SIZE;
    }
    return start_addr;
}

void MMU_free_page(void *vaddr) {
    uint64_t *pml4t = phys_to_virt(get_cr3() & PAGE_MASK);
    uint64_t *pte = get_pte(pml4t, (uint64_t)vaddr, 0);
    if (pte) {
        if (*pte & PTE_PRESENT) {
            void *page_frame = (void*)(*pte & PAGE_MASK);
            MMU_pf_free(page_frame);
        }
        *pte = 0;
        invlpg(vaddr);
    }
}

void MMU_free_pages(void *vaddr, int num) {
    for (int i = 0; i < num; i++) {
        MMU_free_page((void*)((uint64_t)vaddr + i * PAGE_SIZE));
    }
}

void page_fault_handler(struct interrupt_frame* frame) {
    uint64_t fault_address;
    // cr2 contains virtual address of page that faulted
    __asm__ volatile("mov %%cr2, %0" : "=r" (fault_address));
    uint64_t cr3 = get_cr3();
    uint64_t *pml4t = phys_to_virt(cr3 & PAGE_MASK);
    uint64_t *pte = get_pte(pml4t, fault_address, 0);
    
    // check if demand paging
    if (pte && (*pte & PTE_DEMAND_PAGING)) {
        void *page_frame = MMU_pf_alloc();
        if (!page_frame) {
            printk("Out of memory during demand paging\n");
            goto error;
        }
        memset(page_frame, 0, PAGE_SIZE);
        *pte = ((uint64_t)page_frame & PAGE_MASK) | 
               (*pte & ~PTE_DEMAND_PAGING) | 
               PTE_PRESENT;
        invlpg((void*)fault_address);
        return;
    }
error:
    printk("=== PAGE FAULT ===\n");
    printk("Address: 0x%lx\n", fault_address);
    printk("Page table (CR3): 0x%lx\n", cr3);
    printk("Error code: 0x%lx\n", frame->err_code);
    if (!(frame->err_code & 1)) {
        printk("Page not present\n");
    } else {
        printk("Page protection violation\n");
    }
    if (frame->err_code & 2) {
        printk("Write access\n");
    } else {
        printk("Read access\n");
    }
    if (frame->err_code & 4) {
        printk("User mode access\n");
    } else {
        printk("Kernel mode access\n");
    }
    if (frame->err_code & 8) {
        printk("Reserved bits set in page table\n");
    }
    if (frame->err_code & 16) {
        printk("Instruction fetch\n");
    }
    printk("Faulting instruction at: 0x%lx\n", frame->rip);
    printk("Unhandled page fault - halting\n");
    while(1) {
        __asm__ volatile("cli");
        __asm__ volatile("hlt");
    }
}