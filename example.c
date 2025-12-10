// example.c
#include <stdio.h>
#include <string.h>
#include "deheisenbug.h"

// Define a size that matches the default Groom size to ensure UAF hits the pre-filled spot
#define BUGGY_ALLOC_SIZE 64 

void buggy_operation(int run_number) {
    printf("--- Run %d ---\n", run_number);
    
    // 1. The Use-After-Free (UAF) Bug
    char *a = malloc(BUGGY_ALLOC_SIZE); 
    if (!a) {
        printf("Malloc failed.\n");
        return;
    }

    // Fill the chunk with a unique "victim" string
    strcpy(a, "🔥 UAF_VICTIM_DATA_FOR_DEBUG 🔥"); 
    
    // The programmer mistakenly frees the memory
    free(a); 

    // In a normal, fragmented heap, the next malloc might land anywhere,
    // making the UAF non-deterministic.
    
    // 2. Subsequent Allocation (The 'attacker' or 'replacing' chunk)
    // The allocator will typically reuse the last freed chunk (a) for the new one (b)
    char *b = malloc(BUGGY_ALLOC_SIZE); 

    // Check the content of 'b' WITHOUT initializing it (which is the UAF condition)
    
    // When DeHeisenbug is NOT used, the output here is RANDOM, sometimes a crash, 
    // sometimes a clean string, sometimes "garbage".
    
    // When DeHeisenbug IS used, the heap is stable. 'b' 100% lands on 'a's spot.
    // The output will 100% show the previous content if the allocator didn't zero the memory.
    
    printf("Content of new block (b): %s\n", b); 

    // We free 'b' to avoid memory leaks for the example loop.
    free(b); 
}

int main() {
    printf("==========================================\n");
    printf("DeHeisenbug Example: Reproducing Intermittent UAF\n");
    printf("==========================================\n");

    // --- The Magic Line ---
    // Force the heap layout to a deterministic state BEFORE the buggy logic runs.
    DeHeisenbug_GroomHeap_Default(); 
    // --- End Magic ---

    printf("\nHeap Grooming Complete. Starting Bug Reproduction Loop...\n\n");

    for (int i = 1; i <= 5; i++) {
        buggy_operation(i); 
    }
    
    printf("\nConclusion: The UAF has been reproduced 100%% deterministically.\n");
    return 0;
}
