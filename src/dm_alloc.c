#include "dm_alloc.h"

// the current head of mmalloc
static BlockHeader *head = NULL;
const size_t ALIGN = 8;

/**
 * @brief Round up `size` to the closest factor of `align`
 *
 * @param size the number to round up
 * @param align factor to round up
 *
 * @return rounded up size
 */
static inline size_t align_up(size_t size, size_t align)
{

    return (size + (align - 1)) & ~(align - 1);
}

/**
 * @brief Create a block and append it to the list
 *
 * @param mem_ptr current program break pointer
 * @param size size of the block to allocate
 *
 * @returns pointer to the allocated block
 */
BlockHeader *append(void *mem_ptr, size_t size)
{
    BlockHeader *block = (BlockHeader *)mem_ptr; /* tells complier to treat `mem_ptr` as the starting of `BlockHeader` and also tells that data will be stored in the structure of `BlockHeader` */
    block->size = size;                          // payload size only
    block->free = 0;
    block->next = NULL;
    if (!head)
    {
        head = block;
    }
    else
    {
        BlockHeader *curr = head;
        while (curr->next != NULL)
        {
            curr = curr->next;
        }
        curr->next = block;
    }
    return block;
}

/**
 * @brief allocates `block` of memory
 * @param size size of the payload
 *
 * @return ptr to the payload
 */
void *mmalloc(size_t size)
{

    if (size == 0)
        return NULL;

    size_t asize = align_up(size, ALIGN); // aligned size

    // check for free blocks

    BlockHeader *block = find_free(asize);
    if (block)
    {
        block->free = 0;
        return (block + 1);
    }

    // void *prev_brk = sbrk(0); // get current program break
    size_t total_size = sizeof(BlockHeader) + asize;

    void *mem_ptr = sbrk(total_size); // ptr of the current program break and inc by total_size

    if (mem_ptr == (void *)-1)
        return NULL; // abrk failed

    block = append(mem_ptr, asize);

    return (block + 1); /* skips header and returns the ptr to the payload*/
}

void *mcalloc(size_t num, size_t size)
{
    size_t total_size = num * size;
    void *ptr = mmalloc(total_size);
    if (ptr == NULL)
    {
        return NULL; // malloc failed
    }
    // Set all bytes in the allocated block to 0
    memset(ptr, 0, total_size);
    return ptr;
}

void *mrelloc(void *ptr, size_t size)
{
    if (ptr == NULL)
    {
        // realloc(NULL, size) is equivalent to malloc(size)
        return mmalloc(size);
    }
    if (size == 0)
    {
        // realloc(ptr, 0) is equivalent to free(ptr)
        mfree(ptr);
        return NULL;
    }

    BlockHeader *header = (BlockHeader *)ptr - 1;

    if (header->size == size)
    {
        return header + 1;
    }

    if (header->size > size)
    {
        return split_block(header, size);
    }

    if (header->size < size && header->next->free)
    {
        coalesce();
        if (header->size > size)
        {
            return split_block(header, size);
        }
    }

    else
    {
        BlockHeader *new_header = mmalloc(size);
        mfree(ptr);
        return new_header + 1;
    }
}
/**
 * @brief splits large free blocks for new allocations , if feastable
 *
 * @param block the free block for splitting
 * @param size size of the payload
 *
 * @return `block` for allocating data
 */
BlockHeader *split_block(BlockHeader *block, size_t size)
{
    size_t asize = align_up(size, ALIGN);
    // size : required size for the block
    size_t leftover = block->size - asize - sizeof(BlockHeader);

    // the min block size is sizeof(BlockHeader) + ALIGN, if leftover <= no use of splitting
    if (leftover <= sizeof(BlockHeader) + ALIGN)
    {
        // not enough to split, use whole block
        // free the block and use it
        block->free = 0;
        return block;
    }
    // if its splittable
    // create new Blockheader at leftover location

    /* move new_block to skip current `block` header and asize(needed to store data)*/
    BlockHeader *new_block = (BlockHeader *)((char *)(block + 1) + asize);

    new_block->size = leftover;
    new_block->free = 1;
    new_block->next = block->next;

    // update original block
    block->size = asize;
    block->free = 0;
    block->next = new_block;

    return block; // the allocated part
}

/**
 * @brief finds a free block from the pre-allocated blocks.
 * @param size size of the block needed
 *
 * @return ptr of the free block on success, else NULL
 */
BlockHeader *find_free(size_t size)
{
    BlockHeader *curr = head;
    while (curr != NULL)
    {
        if (curr->free)
        {
            if (curr->size == size)
            {
                return curr; // perfect fit
            }
            if (curr->size >= size + sizeof(BlockHeader) + ALIGN)
            {
                return split_block(curr, size); // split
            }
            if (curr->size >= size)
            {
                // block is a bit bigger, but not enough to split
                return curr;
            }
        }

        curr = curr->next;
    }
    return NULL;
}

/**
 * @brief join free blocks
 */
void coalesce()
{
    BlockHeader *curr = head;

    while (curr && curr->next)
    {
        if (curr->free && curr->next->free)
        {
            // merge curr with next
            size_t prev_size = curr->size;
            curr->size += sizeof(BlockHeader) + curr->next->size;
            curr->next = curr->next->next;
            // do not move curr forward — there might be more consecutive free blocks
        }
        else
        {
            curr = curr->next;
        }
    }
}

/**
 * @brief free allocated blocks
 *
 * @param ptr block pointer
 */
void mfree(void *ptr)
{
    if (!ptr)
        return;
    /*
    as in mmalloc, block -1 moves our ptr to
    say 16 bytes backwards to the start of our
    header.
    */
    BlockHeader *block = ((BlockHeader *)ptr) - 1;
    block->free = 1;

    // not so performance friendly
    // memset(ptr, 0, block->size); // write the content t0 0

    // coalescing
    coalesce();
}

/**
 * @brief to print block status, for debug info.
 */
void print_heap()
{
    BlockHeader *curr = head;
    printf("Heap blocks:\n");
    while (curr)
    {
        printf("  Block %p: size=%zu, free=%d, user_ptr=%p\n",
               curr, curr->size, curr->free, (void *)(curr + 1));
        curr = curr->next;
    }
}
