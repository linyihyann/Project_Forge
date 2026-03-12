/* file: src/srv/srv_os.c */
#include "srv_os.h"

#include <stdio.h>  // 需要用到 printf 印出死亡訊息

#include "FreeRTOS.h"
#include "task.h"

/* 🌟 FreeRTOS Hook 宣告 (滿足 MISRA Rule 8.4) */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName);
void vApplicationMallocFailedHook(void);
void vApplicationGetIdleTaskMemory(StaticTask_t** ppxIdleTaskTCBBuffer,
                                   StackType_t** ppxIdleTaskStackBuffer,
                                   uint32_t* pulIdleTaskStackSize);
void vApplicationGetTimerTaskMemory(StaticTask_t** ppxTimerTaskTCBBuffer,
                                    StackType_t** ppxTimerTaskStackBuffer,
                                    uint32_t* pulTimerTaskStackSize);
// ============================================================================
// ⚙️ OS 抽象層 (OSAL) 核心 API 實作
// ============================================================================

void srv_os_init(void)
{
    /* 這裡可以放一些全域 Mutex 或 Semaphore 的預先初始化 */
}

void srv_os_start_scheduler(void)
{
    /* 啟動 FreeRTOS 排程器，這行執行後不會 return，除非 OS 崩潰 */
    vTaskStartScheduler();
}

bool srv_os_create_task(srv_os_task_func_t func, const char* name, uint32_t stack_words, void* arg,
                        uint8_t priority)
{
    TaskHandle_t handle = NULL;

    /* 呼叫 FreeRTOS 原生 API */
    BaseType_t res = xTaskCreate(func, name, stack_words, arg, priority, &handle);

    return (res == pdPASS);
}

void srv_os_delay_ms(uint32_t ms)
{
    /* 將毫秒轉換為 OS Tick 進行阻塞延遲 */
    vTaskDelay(pdMS_TO_TICKS(ms));
}

// ============================================================================
// 🛡️ FreeRTOS 車規級安全攔截網 (Safety Hooks)
// ============================================================================

/* 1. 堆疊溢位攔截 (Stack Overflow Hook) */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName)
{
    (void)xTask;
    (void)printf("\n\n❌ [FATAL ERROR] OS Stack Overflow in task: %s\n", pcTaskName);

    // 實務上這裡會觸發 Crash Dump 並死鎖等待 Watchdog 救援
    while (1)
    {
        // 卡死在這裡，等待看門狗重啟系統
    }
}

/* 2. 記憶體耗盡攔截 (Malloc Failed Hook) */
void vApplicationMallocFailedHook(void)
{
    (void)printf("\n\n❌ [FATAL ERROR] OS Malloc Failed! Out of Heap Memory.\n");

    while (1)
    {
        // 卡死在這裡，等待看門狗重啟系統
    }
}

/* 3. 提供靜態記憶體給 Idle Task (因為開啟了 configSUPPORT_STATIC_ALLOCATION) */
void vApplicationGetIdleTaskMemory(StaticTask_t** ppxIdleTaskTCBBuffer,
                                   StackType_t** ppxIdleTaskStackBuffer,
                                   uint32_t* pulIdleTaskStackSize)
{
    // 使用靜態全域變數，確保記憶體生命週期與系統一樣長
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/* 4. 提供靜態記憶體給 Timer Task */
void vApplicationGetTimerTaskMemory(StaticTask_t** ppxTimerTaskTCBBuffer,
                                    StackType_t** ppxTimerTaskStackBuffer,
                                    uint32_t* pulTimerTaskStackSize)
{
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}