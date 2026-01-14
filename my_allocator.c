#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>

#define ALIGN8(x) (((x) + 7) & ~7)

typedef struct block {
    size_t size;
    int free;
    struct block* next;
} block_t;

#define BLOCK_SIZE sizeof(block_t)

static block_t* free_list = NULL;

/* Find a free block using First Fit */
block_t* find_free_block(block_t** last, size_t size) {
    block_t* curr = free_list;
    while (curr) {
        if (curr->free && curr->size >= size)
            return curr;
        *last = curr;
        curr = curr->next;
    }
    return NULL;
}

/* Request memory from OS */
block_t* request_space(block_t* last, size_t size) {
    block_t* block = mmap(NULL,
                           size + BLOCK_SIZE,
                           PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS,
                           -1, 0);

    if (block == MAP_FAILED)
        return NULL;

    block->size = size;
    block->free = 0;
    block->next = NULL;

    if (last)
        last->next = block;

    return block;
}

/* Split block if large enough */
void split_block(block_t* block, size_t size) {
    if (block->size >= size + BLOCK_SIZE + 8) {
        block_t* new_block = (block_t*)((char*)block + BLOCK_SIZE + size);
        new_block->size = block->size - size - BLOCK_SIZE;
        new_block->free = 1;
        new_block->next = block->next;

        block->size = size;
        block->next = new_block;
    }
}

/* malloc implementation */
void* my_malloc(size_t size) {
    if (size == 0)
        return NULL;

    size = ALIGN8(size);
    block_t* block;

    if (!free_list) {
        block = request_space(NULL, size);
        if (!block) return NULL;
        free_list = block;
    } else {
        block_t* last = free_list;
        block = find_free_block(&last, size);
        if (!block) {
            block = request_space(last, size);
            if (!block) return NULL;
        } else {
            block->free = 0;
            split_block(block, size);
        }
    }

    return (char*)block + BLOCK_SIZE;
}

/* Coalesce adjacent free blocks */
void coalesce() {
    block_t* curr = free_list;
    while (curr && curr->next) {
        if (curr->free && curr->next->free) {
            curr->size += BLOCK_SIZE + curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}

/* free implementation */
void my_free(void* ptr) {
    if (!ptr) return;

    block_t* block = (block_t*)((char*)ptr - BLOCK_SIZE);
    block->free = 1;
    coalesce();
}