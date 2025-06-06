/**
 * @file memory_allocator.c
 * @brief A simple custom dynamic memory allocator.
 *
 * This allocator manages a fixed-size memory pool and uses a free list
 * to keep track of available memory blocks. It demonstrates basic concepts
 * of memory management like block splitting and coalescing (though coalescing
 * is simplified here for brevity).
 *
 * Features:
 * - Fixed-size memory pool.
 * - First-fit allocation strategy.
 * - Block header to store metadata (size and free status).
 * - Simple free list implementation (singly linked list).
 * - Basic error handling.
 *
 * Limitations:
 * - No advanced coalescing of free blocks implemented in this simple version.
 * - No thread safety.
 * - Fixed pool size, cannot grow.
 * - Minimal error checking for brevity.
 * - Potential for external fragmentation.
 */

#include <stdio.h>
#include <stddef.h> // For size_t, NULL
#include <unistd.h> // For sbrk (conceptual usage, actual sbrk might not be available/ideal in all environments)
#include <string.h> // For memcpy (though not strictly used for core logic, useful for testing)
#include <assert.h> // For assertions

// Define the total size of the memory pool
#define POOL_size (1024 * 1024) // 1MB

// Structure for the memory block header
typedef struct BlockHeader {
    size_t size;        // Size of the block (including header)
    int is_free;        // 1 if free, 0 if allocated
    struct BlockHeader *next_free; // Pointer to the next free block in the free list
    // char data[1];    // Flexible array member (or pointer arithmetic) for data
                       // We will use pointer arithmetic to access data part
} BlockHeader;

// Alignment requirement (typically to the size of a pointer or double)
#define ALIGNMENT sizeof(void*)
// Helper to align size
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

// Global pointer to the start of our memory pool
static unsigned char memory_pool[POOL_SIZE];
// Head of the free list
static BlockHeader *free_list_head = NULL;
// Flag to check if allocator is initialized
static int allocator_initialized = 0;

/**
 * @brief Initializes the custom memory allocator.
 * Sets up the initial free block spanning the entire pool.
 */
void my_allocator_init() {
    if (allocator_initialized) {
        // printf("Allocator already initialized.\n");
        return;
    }

    // The first block spans the entire pool
    free_list_head = (BlockHeader *)memory_pool;
    free_list_head->size = POOL_SIZE;
    free_list_head->is_free = 1;
    free_list_head->next_free = NULL; // No other free blocks initially

    allocator_initialized = 1;
    // printf("Custom memory allocator initialized. Pool size: %zu bytes.\n", POOL_SIZE);
    // printf("Initial free block: Addr=%p, Size=%zu\n", (void*)free_list_head, free_list_head->size);
}

/**
 * @brief Allocates a block of memory of at least 'size' bytes.
 * Uses a first-fit strategy.
 * @param size The number of bytes to allocate.
 * @return Pointer to the allocated memory region (user data area), or NULL if allocation fails.
 */
void *my_malloc(size_t size) {
    if (!allocator_initialized) {
        my_allocator_init();
    }

    if (size == 0) {
        return NULL;
    }

    // Calculate total size needed: requested size + header size, then align
    size_t total_needed_size = ALIGN(size + sizeof(BlockHeader));
    if (total_needed_size < sizeof(BlockHeader) + ALIGNMENT) { // Ensure minimum block size
        total_needed_size = sizeof(BlockHeader) + ALIGNMENT;
    }


    BlockHeader *current = free_list_head;
    BlockHeader *prev_free = NULL;

    // First-fit: find the first free block large enough
    while (current != NULL) {
        if (current->is_free && current->size >= total_needed_size) {
            // Found a suitable block
            // Check if we can split the block
            if (current->size >= total_needed_size + sizeof(BlockHeader) + ALIGNMENT) { // Room for another block
                // Split the block
                BlockHeader *new_free_block = (BlockHeader *)((unsigned char *)current + total_needed_size);
                new_free_block->size = current->size - total_needed_size;
                new_free_block->is_free = 1;
                new_free_block->next_free = current->next_free; // Inherit next free link

                current->size = total_needed_size;
                current->is_free = 0; // Mark as allocated
                current->next_free = NULL; // Allocated blocks are not in free list itself in this design

                // Update free list: current is being removed (or part of it)
                if (prev_free) {
                    prev_free->next_free = new_free_block;
                } else {
                    free_list_head = new_free_block;
                }
                // printf("Allocated %zu bytes (total %zu) by splitting. New free block: Addr=%p, Size=%zu\n",
                //        size, total_needed_size, (void*)new_free_block, new_free_block->size);

            } else {
                // Use the entire block (cannot split sufficiently)
                current->is_free = 0;
                
                // Remove from free list
                if (prev_free) {
                    prev_free->next_free = current->next_free;
                } else {
                    free_list_head = current->next_free;
                }
                current->next_free = NULL; // Allocated blocks are not in free list
                // printf("Allocated %zu bytes (total %zu) using entire block. Addr=%p\n",
                //        size, current->size, (void*)current);
            }
            // Return pointer to the data area (after the header)
            return (void *)((unsigned char *)current + sizeof(BlockHeader));
        }
        prev_free = current;
        current = current->next_free;
    }

    // No suitable block found
    // fprintf(stderr, "my_malloc: Failed to allocate %zu bytes. No sufficiently large free block.\n", size);
    return NULL;
}

/**
 * @brief Frees a previously allocated memory block.
 * @param ptr Pointer to the memory block to free (must be one returned by my_malloc).
 */
void my_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    if (!allocator_initialized) {
        // This shouldn't happen if ptr was allocated by my_malloc after init
        // fprintf(stderr, "my_free: Allocator not initialized or invalid pointer.\n");
        return;
    }

    // Get the block header from the user pointer
    BlockHeader *block_to_free = (BlockHeader *)((unsigned char *)ptr - sizeof(BlockHeader));

    // Basic validation: Check if the pointer is within our pool
    if ((unsigned char *)block_to_free < memory_pool ||
        (unsigned char *)block_to_free >= memory_pool + POOL_SIZE) {
        // fprintf(stderr, "my_free: Attempting to free pointer outside of memory pool: %p\n", ptr);
        return;
    }
    
    // Check if block is already freed (simple check, not robust against corruption)
    // A more robust check might involve magic numbers in the header or ensuring it's not already in the free list
    if (block_to_free->is_free) {
        // fprintf(stderr, "my_free: Block at %p already free (or header corrupted).\n", (void*)block_to_free);
        return;
    }


    block_to_free->is_free = 1;

    // Add the block to the head of the free list (simple approach)
    // More advanced would be to keep free list sorted by address for coalescing
    block_to_free->next_free = free_list_head;
    free_list_head = block_to_free;
    
    // printf("Freed block: Addr=%p, Size=%zu. New free list head: %p\n",
    //        (void*)block_to_free, block_to_free->size, (void*)free_list_head);

    // TODO: Implement coalescing: iterate through free list or check adjacent blocks by address.
    // For simplicity, this version doesn't implement coalescing of adjacent free blocks.
    // Proper coalescing would involve:
    // 1. Checking the block physically preceding the freed block.
    // 2. Checking the block physically succeeding the freed block.
    // This requires either iterating all blocks or having bidirectional links / boundary tags.
}

/**
 * @brief Dumps the current state of the memory pool and free list.
 * Useful for debugging.
 */
void dump_memory_map() {
    if (!allocator_initialized) {
        my_allocator_init(); // Ensure it's initialized for dumping even if no allocs/frees yet
    }
    printf("\n--- Memory Pool Map ---\n");
    BlockHeader *current_block = (BlockHeader *)memory_pool;
    unsigned char *pool_end = memory_pool + POOL_SIZE;
    size_t total_bytes_mapped = 0;

    while ((unsigned char *)current_block < pool_end && total_bytes_mapped < POOL_SIZE) {
        printf("Block Addr: %p, Size: %6zu, Status: %s",
               (void *)current_block,
               current_block->size,
               current_block->is_free ? "Free  " : "Alloc ");
        
        if (current_block->is_free) {
             printf(", NextFree: %p", (void*)current_block->next_free);
        }
        printf("\n");

        if (current_block->size == 0) {
            printf("Error: Block with size 0 encountered at %p. Halting dump.\n", (void*)current_block);
            break;
        }
        total_bytes_mapped += current_block->size;
        current_block = (BlockHeader *)((unsigned char *)current_block + current_block->size);
        if (total_bytes_mapped > POOL_SIZE) { // Safety break
             printf("Error: total_bytes_mapped exceeded POOL_SIZE. Header corruption likely.\n");
             break;
        }
    }
    printf("Total bytes mapped: %zu / %d\n", total_bytes_mapped, POOL_SIZE);


    printf("\n--- Free List ---\n");
    BlockHeader *free_block = free_list_head;
    int count = 0;
    while (free_block != NULL) {
        printf("Free Block #%d: Addr: %p, Size: %6zu, NextFree: %p\n",
               count++,
               (void *)free_block,
               free_block->size,
               (void *)free_block->next_free);
        if ((unsigned char*)free_block < memory_pool || (unsigned char*)free_block >= pool_end) {
            printf("Error: Free block %p is outside pool bounds. Halting dump.\n", (void*)free_block);
            break;
        }
        if (!free_block->is_free) {
            printf("Error: Block %p in free list but marked as allocated. Halting dump.\n", (void*)free_block);
            break;
        }
        free_block = free_block->next_free;
        if (count > POOL_SIZE / (sizeof(BlockHeader) + ALIGNMENT) + 5) { // Safety break for cyclic list
            printf("Error: Free list seems too long or cyclic. Halting dump.\n");
            break;
        }
    }
    if (count == 0) {
        printf("Free list is empty.\n");
    }
    printf("-----------------------\n");
}


// Example Usage
int main() {
    printf("Starting custom memory allocator test.\n");
    my_allocator_init(); // Explicitly call, though my_malloc would do it.
    dump_memory_map();

    printf("\nAllocating p1 (100 bytes)...\n");
    void *p1 = my_malloc(100);
    assert(p1 != NULL);
    printf("p1 allocated at %p\n", p1);
    dump_memory_map();

    printf("\nAllocating p2 (200 bytes)...\n");
    void *p2 = my_malloc(200);
    assert(p2 != NULL);
    printf("p2 allocated at %p\n", p2);
    dump_memory_map();
    
    printf("\nAllocating p3 (50 bytes)...\n");
    void *p3 = my_malloc(50);
    assert(p3 != NULL);
    printf("p3 allocated at %p\n", p3);
    // Let's write some data
    strcpy((char*)p3, "hello");
    printf("p3 data: %s\n", (char*)p3);
    dump_memory_map();

    printf("\nFreeing p2...\n");
    my_free(p2);
    p2 = NULL; // Good practice
    dump_memory_map();

    printf("\nAllocating p4 (150 bytes) - should reuse part of p2's space or other free space...\n");
    void *p4 = my_malloc(150);
    assert(p4 != NULL);
    printf("p4 allocated at %p\n", p4);
    dump_memory_map();

    printf("\nFreeing p1...\n");
    my_free(p1);
    p1 = NULL;
    dump_memory_map();

    printf("\nFreeing p3...\n");
    my_free(p3);
    p3 = NULL;
    dump_memory_map();
    
    printf("\nAllocating p5 (800KB) - testing larger allocation...\n");
    void* p5 = my_malloc(800 * 1024);
    assert(p5 != NULL);
    printf("p5 allocated at %p\n", p5);
    dump_memory_map();

    printf("\nFreeing p4...\n");
    my_free(p4);
    p4 = NULL;
    dump_memory_map();

    printf("\nFreeing p5...\n");
    my_free(p5);
    p5 = NULL;
    dump_memory_map();

    printf("\nAttempting to allocate almost full remaining pool...\n");
    // After all frees, most of the pool should be one large block (or a few due to no coalescing)
    // Let's see the free list first
    printf("Checking free list before large allocation:\n");
    BlockHeader *fb = free_list_head;
    size_t largest_free = 0;
    while(fb) {
        if (fb->is_free && fb->size > largest_free) largest_free = fb->size;
        fb = fb->next_free;
    }
    printf("Largest available free block (approx): %zu bytes\n", largest_free);

    if (largest_free > sizeof(BlockHeader)) {
        void *p_large = my_malloc(largest_free - sizeof(BlockHeader) - ALIGNMENT); // Request slightly less
        if(p_large) {
            printf("p_large allocated %zu bytes at %p\n", largest_free - sizeof(BlockHeader) - ALIGNMENT, p_large);
            my_free(p_large);
        } else {
            printf("Failed to allocate p_large even if space seems available.\n");
        }
    }
    dump_memory_map();
    
    printf("\nTest freeing a NULL pointer (should be safe).\n");
    my_free(NULL);
    dump_memory_map();

    printf("\nTest double free (should ideally be caught or handled gracefully - basic check here).\n");
    void* p_double_free_test = my_malloc(10);
    if (p_double_free_test) {
        my_free(p_double_free_test);
        printf("First free done. Attempting second free (expect error or specific handling message):\n");
        my_free(p_double_free_test); // This should trigger the 'already free' check
    }
    dump_memory_map();


    printf("\nAllocator test finished.\n");
    return 0;
}

/**
 * Notes on potential improvements and complexities not covered:
 * 1. Coalescing: Merging adjacent free blocks to reduce fragmentation.
 * - Immediate coalescing: When a block is freed, check its neighbors.
 * - Deferred coalescing: Periodically scan and merge.
 * - Requires ways to find neighbors (e.g., boundary tags, or iterating all blocks).
 * 2. Free List Management:
 * - Sorted free list (by address or size) for faster search or better fit.
 * - More complex data structures for free list (e.g., balanced trees for very large numbers of blocks).
 * 3. Allocation Strategies:
 * - Best-fit: Find the smallest free block that is large enough. Can leave tiny unusable fragments.
 * - Worst-fit: Find the largest free block. Might leave large usable fragments.
 * 4. Thread Safety: Using mutexes to protect shared data structures (free list, pool metadata) in a multithreaded environment.
 * 5. Error Handling: More robust error checking (e.g., magic numbers in headers to detect corruption).
 * 6. Realloc: Implementing `my_realloc` to resize existing allocations.
 * 7. Boundary Tags: Storing metadata at both ends of a block to facilitate coalescing with the previous block.
 */
