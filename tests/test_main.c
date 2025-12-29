#include "dm_alloc.h"
#include <stdio.h>

void test_malloc_free()
{
    printf("\n--- TEST START ---\n");

    void *a = mmalloc(64);
    void *b = mmalloc(128);
    void *c = mmalloc(64);

    printf("After 3 allocations (64,128,64):\n");
    print_heap();

    printf("\nFreeing middle block (b = 128):\n");
    mfree(b);
    print_heap();

    printf("\nAllocating 52 bytes :\n");
    void *d = mmalloc(52);
    print_heap();

    printf("\nAllocating 16 bytes :\n");
    void *e = mmalloc(16);
    print_heap();

    printf("\nFree all blocks:\n");
    mfree(a);
    mfree(c);
    mfree(d);
    mfree(e);
    print_heap();

    printf("--- TEST END ---\n");
}

int main(void)
{
    printf("BLOCK HEADER SIZE : %d bytes\n", sizeof(BlockHeader));
    test_malloc_free();
    return 0;
}
