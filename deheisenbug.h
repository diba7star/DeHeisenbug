// deheisenbug.h

#ifndef DEHEISENBUG_H
#define DEHEISENBUG_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// #define DEBUG_DEHEISENBUG 1 // Uncomment to enable internal logs

#define DEFAULT_GROOM_ITERATIONS 1000
#define DEFAULT_GROOM_SIZE       128
#define DEFAULT_GROOM_PATTERN    0xDE // A clear signature byte (0xDE for DeHeisenbug)

/*
 * @brief Performs Heap Grooming to establish a predictable heap layout.
 * * This function allocates a specific number of chunks of a fixed size 
 * and fills them with a recognizable pattern. This process forces the 
 * memory allocator (e.g., glibc's ptmalloc) into a known, stable state.
 * * This is crucial for reproducing intermittent memory bugs like 
 * Use-After-Free (UAF) and Double-Free.
 * * @param iterations The number of memory blocks to allocate (e.g., 1000).
 * @param size The size of each block in bytes (e.g., 128).
 * @param pattern The 8-bit byte pattern used to fill the memory.
 */
void DeHeisenbug_GroomHeap(size_t iterations, size_t size, unsigned char pattern) {
    // Using static storage to ensure the allocated chunks persist and prevent 
    // compiler optimization from removing the allocations.
    static void** allocated_chunks = NULL;
    
    // Prevent re-execution if already groomed
    if (allocated_chunks != NULL) return; 

#ifdef DEBUG_DEHEISENBUG
    printf("[DeHeisenbug] Grooming heap with %zu blocks of size %zu, pattern 0x%02X...\n", 
           iterations, size, pattern);
#endif

    // Allocate storage for pointers (this storage will not be freed)
    allocated_chunks = (void**)malloc(iterations * sizeof(void*));
    if (allocated_chunks == NULL) {
        fprintf(stderr, "[DeHeisenbug ERROR] Failed to allocate internal storage for pointers.\n");
        return;
    }

    // The Grooming Phase: Fill the heap with known patterns
    for (size_t i = 0; i < iterations; i++) {
        void* p = malloc(size);
        if (p == NULL) {
            fprintf(stderr, "[DeHeisenbug WARNING] Malloc failed at iteration %zu. Stopping groom.\n", i);
            break;
        }
        
        // Fill memory with pattern. Adding a small offset for better tracking.
        unsigned char current_pattern = pattern + (i % 8); 
        memset(p, current_pattern, size); 
        
        allocated_chunks[i] = p;
    }

    // Crucial Step: The allocated blocks are intentionally NOT freed. 
    // They anchor the heap's structure for subsequent (buggy) allocations.
    
#ifdef DEBUG_DEHEISENBUG
    printf("[DeHeisenbug] Heap grooming finished. %zu blocks anchored.\n", iterations);
#endif
}

// Convenience wrapper with default values
void DeHeisenbug_GroomHeap_Default() {
    DeHeisenbug_GroomHeap(DEFAULT_GROOM_ITERATIONS, DEFAULT_GROOM_SIZE, DEFAULT_GROOM_PATTERN);
}


#endif // DEHEISENBUG_H
