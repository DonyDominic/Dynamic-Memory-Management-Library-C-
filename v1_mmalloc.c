/* primitive_malloc.c
 *
 * A minimal "malloc" that just calls sbrk() and returns aligned memory.
 * NO free(), NO headers — educational only.
 */

#include <unistd.h> // sbrk
#include <stdint.h> //uintptr_t
#include <stddef.h> // size_t
#include <stdio.h> 
#include <string.h> //memset (for tests)
#include <errno.h>

static inline size_t align_up(size_t size,size_t align){
    // the mem returned must be powers of 8 .
    //ex) if we want 13 byte,return multiple of 8 rounded up, 16.
    /*
    simplified logic :
        size_t remainder = size % align;
        if (remainder == 0) return size;
        return size + (align - remainder);
    
    performace_oriented:
    size + (align - 1) : used to round up
    ~(align - 1) : a mask,this mask forces the result
    to drop the last 3 bits, making it a multiple of 8.
    */
   return (size+(align-1)) & ~(align-1);
}

void *mmalloc(size_t size){
    /*
    void *prev_brk = sbrk(0);

    sbrk(0) returns the current “program break”
    — the top of the heap.

    void *res = sbrk(asize);

    sbrk(asize) asks the OS: “move the program break up
    by asize bytes”.
    */
    if (size == 0) return NULL;
    const size_t ALIGN = 8;
    size_t asize = align_up(size , ALIGN);
    void *prev_brk = sbrk(0); // get current program break
    void *res = sbrk(asize); // ask kernal for `asize` bytes

    if (res == (void*) -1) return NULL; // abrk failed
    return res; // the mem ptr
}

int main(void){
    printf("Testing primitive my_malloc() using sbrk()\n");

    // current break
    void* start = sbrk(0);
    printf("Initial program break: %p\n", start);

    // Allocate a few blocks and print ptr's

    void* p1 = mmalloc(13);
    printf("p1 = %p (requested 13 bytes)\n", p1);

    void *p2 = mmalloc(100);
    printf("p2 = %p (requested 100 bytes)\n", p2);

    void *p3 = mmalloc(1);
    printf("p3 = %p (requested 1 byte)\n", p3);

    // Show that addresses grow (heap grows upward)
    // how much mem reserved b/w p1 and p2
    printf("d(p2-p1) = %ld bytes\n", (long)((uintptr_t)p2 - (uintptr_t)p1));
    // how much mem reserved b/w p3 and p2
    printf("d(p3-p2) = %ld bytes\n", (long)((uintptr_t)p3 - (uintptr_t)p2));

    // Use memory : writing to mem
    memset(p2, 0x42, 100);
    printf("Wrote to p2 (100 bytes) ok\n");

    // Show current break
    void *end = sbrk(0);
    printf("Final program break: %p\n", end);
    printf("Total heap increase: %ld bytes\n", (long)((uintptr_t)end - (uintptr_t)start));

    return 0;
}