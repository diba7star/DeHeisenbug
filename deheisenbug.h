// deheisenbug.h

#ifndef DEHEISENBUG_H
#define DEHEISENBUG_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// #define DEBUG_DEHEISENBUG 1 // برای فعال کردن لاگ‌های داخلی

#define DEFAULT_GROOM_ITERATIONS 1000
#define DEFAULT_GROOM_SIZE       128

/*
 * @brief تابع اصلی برای اجرای تکنیک Heap Grooming.
 * * @param iterations تعداد چانک‌هایی که باید allocate شوند.
 * @param size اندازه هر چانک (به بایت).
 * @param pattern الگوی 8-بیتی برای پر کردن چانک‌ها.
 */
void DeHeisenbug_GroomHeap(size_t iterations, size_t size, unsigned char pattern) {
    // ما یک آرایه استاتیک از پوینترها را برای جلوگیری از بهینه‌سازی کامپایلر نگه می‌داریم.
    // و همچنین برای اینکه سیستم‌عامل مجبور به نگه‌داشتن این حافظه باشد.
    static void** allocated_chunks = NULL;
    
    // اگر قبلاً اجرا شده باشد، دوباره اجرا نمی‌کنیم (اختیاری برای سادگی)
    if (allocated_chunks != NULL) return; 

#ifdef DEBUG_DEHEISENBUG
    printf("[DeHeisenbug] Grooming heap with %zu blocks of size %zu...\n", iterations, size);
#endif

    // تخصیص آرایه‌ای برای نگهداری پوینترها
    allocated_chunks = (void**)malloc(iterations * sizeof(void*));
    if (allocated_chunks == NULL) {
        fprintf(stderr, "[DeHeisenbug ERROR] Failed to allocate storage for pointers.\n");
        return;
    }

    // مرحله Grooming: پر کردن هیپ
    for (size_t i = 0; i < iterations; i++) {
        void* p = malloc(size);
        if (p == NULL) {
            fprintf(stderr, "[DeHeisenbug WARNING] Malloc failed at iteration %zu. Stopping groom.\n", i);
            break;
        }
        
        // پر کردن حافظه با الگوی ثابت + اندکی تنوع (برای دیباگ بهتر)
        unsigned char current_pattern = pattern + (i % 8); 
        memset(p, current_pattern, size); 
        
        allocated_chunks[i] = p;
    }

    // نکته کلیدی: ما این چانک‌ها را free نمی‌کنیم. 
    // این کار تضمین می‌کند که هیپ به یک حالت ثابت "پرشده" برسد
    // و تخصیص‌های بعدی ما (که باگ دارند) با ثبات بیشتری انجام شوند.
    
#ifdef DEBUG_DEHEISENBUG
    printf("[DeHeisenbug] Heap grooming finished. %zu blocks anchored.\n", iterations);
#endif
}

#endif // DEHEISENBUG_H
