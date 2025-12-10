# 🐜 DeHeisenbug
**Turning Intermittent Memory Bugs into 100% Reproducible Bugs.**

## 💡 Motivation: The Heisenbug Problem
If you've spent any time debugging C or C++ projects, you've encountered them: **Heisenbugs**. These are memory corruption issues (like Use-After-Free, Double-Free, or subtle Race Conditions) that only crash "sometimes" (e.g., 1 out of 50 runs). They are impossible to debug because the act of observing them (e.g., running in GDB) often changes the heap layout, making the bug disappear.

The root cause of this intermittency is the **non-deterministic state of the memory heap** at runtime.

## ✨ The DeHeisenbug Solution: Heap Grooming for Debugging
`DeHeisenbug` solves this by applying **Heap Grooming** techniques—a powerful methodology usually reserved for low-level exploit development—to the debugging workflow.

We force the memory allocator (like `ptmalloc` in glibc) into a **stable, predictable state** before the buggy code executes. 

### How It Works:
1.  **Anchoring:** Before running your main logic, `DeHeisenbug_GroomHeap` allocates a large, fixed number of chunks (`N` blocks of `X` size).
2.  **Pre-filling:** It fills these chunks with a recognizable pattern (default `0xDE`).
3.  **Deterministic State:** By saturating the memory allocator's caches (like `tcache` and `fastbins`), we ensure that subsequent, problematic allocations in your program are highly likely to land in a predictable location—often exactly where the previous `free` operation left behind data (the UAF spot).

**Result:** A bug that previously happened 2% of the time, now happens **100% of the time**, making it trivial to inspect in a debugger.

## 🚀 Usage

`DeHeisenbug` is a single, header-only C file.

1.  Clone the repository or download `deheisenbug.h`.
2.  Include it in your main application file.
3.  Call the function *at the very beginning* of your program's execution, before any logic that might trigger the intermittent bug.

### Simple Example: Reproducing a Use-After-Free (UAF)

```c
#include "deheisenbug.h"
// ... (rest of your includes)

int main() {
    // ------------------------------------------
    // 1. ANCHOR THE HEAP: Force a stable memory layout
    DeHeisenbug_GroomHeap_Default(); 
    // ------------------------------------------

    // Now, your buggy logic is 100% deterministic
    buggy_function_with_uaf(); 
    
    return 0;
}
