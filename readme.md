# 🧠 Custom Dynamic Memory Allocator
A deterministic memory allocation library implemented in C, replacing the standard `malloc` family using the `sbrk()` system call.

## 🚀 Features
- **mmalloc:** Allocates a block of memory on the heap using a **First-Fit** search algorithm.
- **mfree:** Marks blocks as free and manages the free-list for reuse.
- **mcalloc:** Allocates and zeros-out memory.
- **mrealloc:** Resizes existing memory blocks efficiently.

## 🛠️ Technical Implementation
This allocator manages a **Singly Linked List** on the heap. Each memory chunk is preceded by a `metadata` header:

```c
typedef struct BlockHeader
{
    size_t size;              // size of user data
    int free;                 // 1 if free, 0 if used
    struct BlockHeader *next; // next block in linked list
} BlockHeader;
```