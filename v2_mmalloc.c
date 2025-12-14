/* v2_primitive_malloc.c
 *
 * free(),  headers — implemented
 * 
 * create a global linked list of alocated blocks
 * search for free block
 * if not, request for more mem with sbrk()
 * 
 * for freeing,
 * get block's head, and mark it free
 *  (opt) merge with neighbour free blocks
 * 
 * (opt) splitting blocks
 * if i have 200 blocks and one request 20 blocks
 * divide the 200 blocks into smaller ones
 * 
 * can store 20 byte in a 200 byte block, waste's 180 bytes.
 * 
 * Coalescing:
 * Merge neighboring free blocks when freeing,
 *  reducing fragmentation.
 * 
 * 
 * Alignment improvements:
 * Ensure the user pointer is always 8-byte
 * aligned, even after splitting.
 */

#include <unistd.h> // sbrk
#include <stdint.h> //uintptr_t
#include <stddef.h> // size_t
#include <stdio.h> 
#include <string.h> //memset (for tests)
#include <errno.h>

/**
 * @brief This is the whole allocated memory acting as a linked list.
 * @param size  the total memory size for data allocation.
 * @param free whether the block is in use.
 * @param next pointer to the next memory block.
 *
 * <user payload> stored here after the header.
 */
typedef struct BlockHeader {
    size_t size;              // size of user data
    int free;                 // 1 if free, 0 if used
    struct BlockHeader* next; // next block in linked list
} BlockHeader;

static BlockHeader* head = NULL;
const size_t ALIGN = 8;

static inline size_t align_up(size_t size,size_t align){

   return (size+(align-1)) & ~(align-1);
}

BlockHeader* append(void* res, size_t size){
    BlockHeader* block = (BlockHeader*) res;
    block->size=size; // payload size only
    block->free=0;
    block->next=NULL;
    if (!head){
        head=block;
    }
    else{
        BlockHeader* curr = head;
    while (curr->next!=NULL)
    {
        curr=curr->next;
    }
    curr->next=block;
    }
    return block;
}
BlockHeader* split_block(BlockHeader* block,size_t size){
    size_t asize=align_up(size,ALIGN);
    // size : required size for the block
    size_t leftover=block->size-asize-sizeof(BlockHeader);

    if (leftover <= sizeof(BlockHeader)+ALIGN){
        // not enough to split, use whole block
        printf("leftover %d bytes not enought for splitting\n",leftover);
        block->free=0;
        return block;
    }

    // create new Blockheader at leftover location
    BlockHeader* new_block=(BlockHeader*)((char*)(block+1)+asize);

    new_block->size=leftover;
    new_block->free=1;
    new_block->next=block->next;

    // update original block
    block->size=asize;
    block->free=0;
    block->next=new_block;

    // log info
    if (leftover < sizeof(BlockHeader) + ALIGN) {
        printf("leftover %zu bytes too small for further splitting, still marked free\n", leftover);
    } else {
        printf("Created leftover block of size %zu bytes, free=1\n", leftover);
    }

    return block; // the allocated part
}

BlockHeader* find_free(size_t size){
    BlockHeader* curr = head;
    int min_block_size = sizeof(BlockHeader)+ALIGN;
    while (curr != NULL)
    {
       if (curr->free) {
            if (curr->size == size) {
                printf("A perfect-fit block found for size %d byte\n",size);
                return curr; // perfect fit
            }
            if (curr->size >= size + sizeof(BlockHeader) + ALIGN) {
                printf("Trying to split a block for %d bytes\n",size);
                return split_block(curr, size); // split
            }
            if (curr->size >= size) {
                // block is a bit bigger, but not enough to split
                printf("Spitting didnt worked, taking the whole block\n");
                return curr;
            }
        }

        
        curr=curr->next; 
        
    }
    return NULL;
}
void *mmalloc(size_t size){

    if (size == 0) return NULL;
    // give mem metadata with mem
    size_t asize = align_up(size , ALIGN);

    // check for free blocks

    BlockHeader* block = find_free(asize);
    if (block)
    {
        block->free = 0;
        return (block+1);
    }
    

    // void *prev_brk = sbrk(0); // get current program break
    size_t total_size = sizeof(BlockHeader) + asize;

    void *res = sbrk(total_size); // ask kernal for `asize` bytes

    if (res == (void*) -1) return NULL; // abrk failed

    block = append(res,asize);
    /*
    ptr math:
    
    block is of BlockHeader*, which is say 16 bytes
    when block+1, it moves 16 bytes front
    skipping our header
    */
    return (block+1);
}

void coalesce() {
    BlockHeader* curr = head;

    while (curr && curr->next) {
        if (curr->free && curr->next->free) {
            // merge curr with next
            size_t prev_size = curr->size;
            curr->size += sizeof(BlockHeader) + curr->next->size;
            curr->next = curr->next->next;
            printf("Merged a block from %zu to %zu bytes\n", prev_size, curr->size);
            // do not move curr forward — there might be more consecutive free blocks
        } else {
            curr = curr->next;
        }
    }
}


void mfree(void *ptr){
    if (!ptr) return;
    /*
    as in mmalloc, block -1 moves our ptr to
    say 16 bytes backwards to the start of our
    header. 
    */
    BlockHeader* block = ((BlockHeader*)ptr) -1;
    block->free = 1;

    // not so performance friendly
    // memset(ptr, 0, block->size); // write the content t0 0

    //coalescing
    coalesce();

}

void print_heap() {
    BlockHeader* curr = head;
    printf("Heap blocks:\n");
    while (curr) {
        printf("  Block %p: size=%zu, free=%d, user_ptr=%p\n",
               curr, curr->size, curr->free, (void*)(curr + 1));
        curr = curr->next;
    }
}
void test_malloc_free() {
    printf("\n--- TEST START ---\n");

    void* a = mmalloc(64);
    void* b = mmalloc(128);
    void* c = mmalloc(64);

    printf("After 3 allocations (16,32,24):\n");
    print_heap();

    printf("\nFreeing middle block (b = 32):\n");
    mfree(b);
    print_heap();

    printf("\nAllocating 8 bytes (should reuse b's space with split):\n");
    void* d = mmalloc(52);
    print_heap();

    printf("\nAllocating 16 bytes (should reuse leftover from split or new block):\n");
    void* e = mmalloc(16);
    print_heap();

    printf("\nFree all blocks:\n");
    mfree(a);
    mfree(c);
    mfree(d);
    mfree(e);
    print_heap();

    printf("--- TEST END ---\n");
}


int main(void) {
    printf("MIN BLOCK SIZE : %d bytes\n",sizeof(BlockHeader)+ALIGN);
    test_malloc_free();
    return 0;
}
