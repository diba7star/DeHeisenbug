// مثال: example.c
#include "deheisenbug.h" 

void buggy_function() {
    // 1. باگ: Use-After-Free که فقط گاهی رخ می‌دهد
    char *a = malloc(64); 
    strcpy(a, "SECRET_DATA"); 
    free(a); 

    // 2. حالا، یک تخصیص دیگر با همان اندازه (یا نزدیک به آن) انجام می‌دهیم.
    // معمولاً این تخصیص رندوم است، اما با Grooming ما...
    char *b = malloc(64); 

    // اگر باگ رخ دهد، b به آدرس آزاد شده a اشاره می‌کند!
    printf("Content of b: %s\n", b); 
}

int main() {
    // --- جادو اینجا اتفاق می‌افتد ---
    DeHeisenbug_GroomHeap(1000, 128, 0xDE); // تنظیم هیپ با الگوی 0xDE
    // ------------------------------------

    for (int i = 0; i < 10; i++) {
        printf("Run %d: ", i);
        buggy_function(); 
    }
    return 0;
}

// نتیجه بدون DeHeisenbug: گاهی کرش یا خروجی تمیز.
// نتیجه با DeHeisenbug: 10/10 خروجی "SECRET_DATA" (اگر UAF رخ دهد).
