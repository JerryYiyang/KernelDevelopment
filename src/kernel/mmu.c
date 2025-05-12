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
                    // overlaps the beginning - adjust start address
                    memory_regions[j].start = overlap_end;
                } 
                else if (overlap_end == memory_regions[j].end) {
                    // overlaps the end - adjust end address
                    memory_regions[j].end = overlap_start;
                } 
                else if (num_memory_regions < MAX_MEMORY_REGIONS) {
                    // splits the region in two - create a new region
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

void MMU_test(void) {
    printk("Testing page frame allocator\n");
    MMU_print_memory_map();
    void *page1 = MMU_pf_alloc();
    void *page2 = MMU_pf_alloc();
    void *page3 = MMU_pf_alloc();
    printk("Allocated pages: 0x%p, 0x%p, 0x%p\n", page1, page2, page3);
    uint32_t *ptr1 = (uint32_t *)page1;
    uint32_t *ptr2 = (uint32_t *)page2;
    uint32_t *ptr3 = (uint32_t *)page3;
    size_t page_ints = PAGE_SIZE / sizeof(uint32_t);
    for (size_t i = 0; i < page_ints; i++) {
        ptr1[i] = 0xDEADBEEF + i;
        ptr2[i] = 0xCAFEBABE + i;
        ptr3[i] = 0xFEEDFACE + i;
    }
    int errors = 0;
    for (size_t i = 0; i < page_ints; i++) {
        if (ptr1[i] != 0xDEADBEEF + i) errors++;
        if (ptr2[i] != 0xCAFEBABE + i) errors++;
        if (ptr3[i] != 0xFEEDFACE + i) errors++;
    }
    if (errors > 0) {
        printk("ERROR: Found %d errors in page contents\n", errors);
    } else {
        printk("All page contents verified successfully\n");
    }
    MMU_pf_free(page1);
    MMU_pf_free(page2);
    MMU_pf_free(page3);
    printk("Pages freed\n");
    void *page4 = MMU_pf_alloc();
    void *page5 = MMU_pf_alloc();
    void *page6 = MMU_pf_alloc();
    printk("Reallocated pages: 0x%p, 0x%p, 0x%p\n", page4, page5, page6);
    printk("Expected (in reverse): 0x%p, 0x%p, 0x%p\n", page3, page2, page1);
    MMU_pf_free(page4);
    MMU_pf_free(page5);
    MMU_pf_free(page6);
    MMU_print_memory_map();
    printk("Page frame allocator test complete\n");
}

void MMU_stress_test(void) {
    printk("Running page frame allocator stress test\n");
    const int max_pages = 100000;
    void *pages[max_pages];
    int count = 0;
    while (count < max_pages) {
        void *page = MMU_pf_alloc();
        if (page == NULL) {
            break;
        }
        pages[count] = page;
        uint32_t *ptr = (uint32_t *)page;
        uint32_t pattern = (uint32_t)(uint64_t)page;
        size_t page_ints = PAGE_SIZE / sizeof(uint32_t);
        for (size_t i = 0; i < page_ints; i++) {
            ptr[i] = pattern + i;
        }
        count++;
    }
    int errors = 0;
    for (int i = 0; i < count; i++) {
        uint32_t *ptr = (uint32_t *)pages[i];
        uint32_t pattern = (uint32_t)(uint64_t)pages[i];
        size_t page_ints = PAGE_SIZE / sizeof(uint32_t);
        for (size_t j = 0; j < page_ints; j++) {
            if (ptr[j] != pattern + j) {
                if (errors < 10) {
                    printk("Error at page 0x%p, offset %ld: expected 0x%lx, got 0x%x\n", 
                           pages[i], j * sizeof(uint32_t), pattern + j, ptr[j]);
                }
                errors++;
                break;
            }
        }
    }
    for (int i = 0; i < count; i++) {
        MMU_pf_free(pages[i]);
    }
    printk("Stress test complete\n");
}