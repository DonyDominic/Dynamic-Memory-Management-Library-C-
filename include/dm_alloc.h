#if !defined(DMALLOC)
#define DMALLOC

#include <unistd.h> // sbrk
#include <stdint.h> //uintptr_t
#include <stddef.h> // size_t
#include <stdio.h>
#include <string.h> //memset 
#include <errno.h>

/**
 * @brief This is the whole allocated memory acting as a linked list.
 * @param size  the total memory size for data allocation.
 * @param free whether the block is in use.
 * @param next pointer to the next memory block.
 *
 * <user payload> stored here after the header.
 */
typedef struct BlockHeader
{
    size_t size;              // size of user data
    int free;                 // 1 if free, 0 if used
    struct BlockHeader *next; // next block in linked list
} BlockHeader;

BlockHeader *append(void *mem_ptr, size_t size);
void *mmalloc(size_t size);
void *mcalloc(size_t num, size_t size);
void *mrelloc(void *ptr, size_t size);
BlockHeader *split_block(BlockHeader *block, size_t size);
BlockHeader *find_free(size_t size);
void coalesce();
void mfree(void *ptr);
void print_heap();

#endif // DMALLOC

