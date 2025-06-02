#ifndef KMALLOC_H
#define KMALLOC_H

#include <stddef.h>
#include <stdint.h>

#define KMALLOC_POOL_SIZES 6
#define KMALLOC_MIN_POOL_SIZE 32
#define KMALLOC_MAX_POOL_SIZE 2048

struct FreeList {
    struct FreeList *next;
};

struct KmallocPool {
    int max_size;
    int avail;
    struct FreeList *head;
};

// acts as metadata
struct KmallocExtra {
    struct KmallocPool *pool;
    size_t size;
};

extern void kfree(void *addr);
extern void *kmalloc(size_t size);
extern void kmalloc_init(void);

#endif