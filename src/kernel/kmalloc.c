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
        printk("ERROR: Failed to allocate page for pool expansion\n");
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
    printk("\n======== Kernel Heap Stats ========\n");
    for (int i = 0; i < KMALLOC_POOL_SIZES; i++) {
        printk("Pool %d bytes: %d free blocks\n",
               (int)pools[i].max_size, pools[i].avail);
    }
    printk("===================================\n\n");
}