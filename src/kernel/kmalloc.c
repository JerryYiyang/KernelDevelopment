#include "kmalloc.h"
#include "mmu.h"
#include "printk.h"
#include "string.h"

static const size_t pool_sizes[KMALLOC_POOL_SIZES] = {32, 64, 128, 512, 1024, 2048};
static struct KmallocPool pools[KMALLOC_POOL_SIZES];
static int kmalloc_initialized = 0;

void kmalloc_init(void) {
    for (int i = 0; i < KMALLOC_POOL_SIZES; i++) {
        pools[i].max_size = pool_sizes[i];
        pools[i].avail = 0;
        pools[i].head = NULL;
    }
    kmalloc_initialized = 1;
    printk("Kernel heap allocator initialized\n");
}

static struct KmallocPool *find_pool(size_t size) {
    for (int i = 0; i < KMALLOC_POOL_SIZES; i++) {
        if (size <= pools[i].max_size) {
            return &pools[i];
        }
    }
    return NULL;
}

static int expand_pool(struct KmallocPool *pool) {
    void *page = MMU_alloc_page();
    if (!page) {
        printk("ERROR: Failed to allocate page for pool expansion, out of memory\n");
        return -1;
    }
    *((volatile char *)page) = 0;
    int blocks_per_page = PAGE_SIZE / pool->max_size;
    char *block_ptr = (char *)page;
    for (int i = 0; i < blocks_per_page; i++) {
        struct FreeList *free_block = (struct FreeList *)block_ptr;
        free_block->next = pool->head;
        pool->head = free_block;
        pool->avail++;
        block_ptr += pool->max_size;
    }
    printk("Expanded pool (size %d): added %d blocks\n", (int)pool->max_size, blocks_per_page);
    return 0;
}

void *kmalloc(size_t size) {
    if (!kmalloc_initialized) {
        kmalloc_init();
    }
    if (size == 0) {
        return NULL;
    }
    
    size_t total_size = size + sizeof(struct KmallocExtra);
    struct KmallocPool *pool = find_pool(total_size);
    if (pool) {
        if (pool->avail == 0) {
            if (expand_pool(pool) < 0) {
                return NULL;
            }
        }
        struct FreeList *free_block = pool->head;
        pool->head = free_block->next;
        pool->avail--;
        struct KmallocExtra *extra = (struct KmallocExtra *)free_block;
        extra->pool = pool;
        extra->size = size;
        return (char *)extra + sizeof(struct KmallocExtra);
    } else {
        // allocation too large for pools, use raw pages
        int pages_needed = (total_size + PAGE_SIZE - 1) / PAGE_SIZE;
        void *pages = MMU_alloc_pages(pages_needed);
        if (!pages) {
            printk("ERROR: Failed to allocate %d pages for large allocation\n", pages_needed);
            return NULL;
        }
        *((volatile char *)pages) = 0;
        struct KmallocExtra *extra = (struct KmallocExtra *)pages;
        extra->pool = NULL; // raw page allocation
        extra->size = size;
        printk("Large allocation: %lu bytes (%d pages)\n", size, pages_needed);
        return (char *)extra + sizeof(struct KmallocExtra);
    }
}

void kfree(void *addr) {
    if (!addr) {
        printk("kfree: NULL pointer\n");
        return;
    }
    struct KmallocExtra *extra = (struct KmallocExtra *)((char *)addr - sizeof(struct KmallocExtra));
    if (extra->pool) {
        struct FreeList *free_block = (struct FreeList *)extra;
        free_block->next = extra->pool->head;
        extra->pool->head = free_block;
        extra->pool->avail++;
    } else {
        size_t total_size = extra->size + sizeof(struct KmallocExtra);
        int pages_to_free = (total_size + PAGE_SIZE - 1) / PAGE_SIZE;
        void *page_start = (void *)((uint64_t)extra & ~(PAGE_SIZE - 1));
        MMU_free_pages(page_start, pages_to_free);
    }
}

void kmalloc_print_stats(void) {
    if (!kmalloc_initialized) {
        printk("Kernel heap allocator not initialized\n");
        return;
    }
    printk("\n======== Kernel heap stats ========\n");
    for (int i = 0; i < KMALLOC_POOL_SIZES; i++) {
        printk("Pool %d bytes: %d free blocks\n",
               (int)pools[i].max_size, pools[i].avail);
    }
    printk("===================================\n\n");
}

void kmalloc_test(void) {
    printk("\n======== Kmalloc basic test ========\n");
    void *ptrs[6];
    size_t test_sizes[] = {16, 48, 96, 400, 800, 1500}; 
    for (int i = 0; i < 6; i++) {
        printk("Allocating %lu bytes (pool %d)...\n", test_sizes[i], i);
        ptrs[i] = kmalloc(test_sizes[i]);
        if (ptrs[i]) {
            printk("Successful allocation: ptr=%p\n", ptrs[i]);
            char *cptr = (char *)ptrs[i];
            for (size_t j = 0; j < test_sizes[i]; j++) {
                cptr[j] = ((char*)&(uint32_t){0xDEADBEEF})[j % 4];
            }
        } else {
            printk("Allocation failed\n");
        }
    }
    kmalloc_print_stats();
    printk("\nVerifying memory patterns\n");
    for (int i = 0; i < 6; i++) {
        if (ptrs[i]) {
            char *cptr = (char *)ptrs[i];
            int errors = 0;
            for (size_t j = 0; j < test_sizes[i]; j++) {
                if (cptr[j] != ((char*)&(uint32_t){0xDEADBEEF})[j % 4]) {
                    errors++;
                }
            }
            printk("Pool %d pattern check: %d errors)\n", i, errors);
        }
    }
    for (int i = 0; i < 6; i++) {
        if (ptrs[i]) kfree(ptrs[i]);
    }

    printk("\nTesting large allocations\n");
    void *large1 = kmalloc(4096);
    void *large2 = kmalloc(8192);
    void *large3 = kmalloc(12000);
    printk("Large alloc 4KB: %p\n", large1);
    printk("Large alloc 8KB: %p\n", large2);
    printk("Large alloc 12KB: %p\n", large3);
    kfree(large1);
    kfree(large2);
    kfree(large3);

    printk("\nEdge cases\n");
    void *zero_alloc = kmalloc(0);
    printk("Zero alloc, should be NULL: %p\n", zero_alloc);
    void *one_byte = kmalloc(1);
    printk("One byte alloc: %p\n", one_byte);
    kfree(one_byte);

    printk("\nPool expansion test\n");
    void *many_ptrs[200];
    int alloc_count = 0;
    for (int i = 0; i < 200; i++) {
        many_ptrs[i] = kmalloc(32);
        if (many_ptrs[i]) {
            alloc_count++;
        } else {
            printk("Allocation failed at index %d\n", i);
            break;
        }
    }
    printk("Successfully allocated %d blocks of 32 bytes\n", alloc_count);
    kmalloc_print_stats();
    for (int i = 0; i < alloc_count; i++) kfree(many_ptrs[i]);
}

void kmalloc_stress_test(void) {
    printk("\n======== Kmalloc stress test ========\n");
    size_t test_sizes[] = {16, 48, 96, 400, 800, 1500};
    const int ALLOCS = 350;
    void *stress_ptrs[ALLOCS];
    int successful_allocs = 0;
    for (int i = 0; i < ALLOCS; i++) {
        size_t size = test_sizes[i % 6];
        stress_ptrs[i] = kmalloc(size);
        if (stress_ptrs[i]) successful_allocs++;
        if (i % 50 == 0) {
            printk("Progress: %d/%d\n", i, ALLOCS);
        }
    }
    printk("Successfully allocated %d/%d blocks\n", successful_allocs, ALLOCS);
    kmalloc_print_stats();
    for (int i = 0; i < ALLOCS; i++) {
        if (stress_ptrs[i]) kfree(stress_ptrs[i]);
    }
    printk("All blocks freed\n");
}