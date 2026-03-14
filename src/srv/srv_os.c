/* file: src/srv/srv_os.c */
#include "srv_os.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

/* MISRA 修正 9.3: 移除部分初始化，交由 BSS 清零與 init 函數明確賦值 */
static TaskHandle_t srv_task_handles[TASK_ID_MAX_COUNT];

/* Hook 宣告 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName);
void vApplicationMallocFailedHook(void);
void vApplicationGetIdleTaskMemory(StaticTask_t** ppxIdleTaskTCBBuffer,
                                   StackType_t** ppxIdleTaskStackBuffer,
                                   uint32_t* pulIdleTaskStackSize);
void vApplicationGetTimerTaskMemory(StaticTask_t** ppxTimerTaskTCBBuffer,
                                    StackType_t** ppxTimerTaskStackBuffer,
                                    uint32_t* pulTimerTaskStackSize);

/* OS API 實作 */
void srv_os_init(void)
{
    /* 🌟 MISRA 修正 9.3：明確、完整地初始化陣列，拒絕隱式行為 */
    for (uint32_t i = 0U; i < (uint32_t)TASK_ID_MAX_COUNT; i++)
    {
        srv_task_handles[i] = NULL;
    }
}

void srv_os_start_scheduler(void)
{
    vTaskStartScheduler();
}

bool srv_os_create_task(os_task_id_t id, srv_os_task_func_t func, const char* name,
                        uint32_t stack_words, void* arg, uint8_t priority)
{
    /* 🌟 MISRA 修正 15.6：強制加上大括號 */
    if (id >= TASK_ID_MAX_COUNT)
    {
        return false;
    }

    TaskHandle_t handle = NULL;
    BaseType_t res = xTaskCreate(func, name, stack_words, arg, priority, &handle);

    if (res == pdPASS)
    {
        srv_task_handles[id] = handle;
        return true;
    }
    return false;
}

os_status_t srv_os_get_task_metric(os_task_id_t id, os_task_metric_t* out_metric)
{
    /* 🌟 MISRA 修正 15.6：強制加上大括號 */
    if ((id >= TASK_ID_MAX_COUNT) || (out_metric == NULL))
    {
        return OS_ERR_PARAM;
    }

    TaskHandle_t handle = srv_task_handles[id];

    /* 🌟 MISRA 修正 15.6：強制加上大括號 */
    if (handle == NULL)
    {
        return OS_ERR_NOT_FOUND;
    }

    out_metric->stack_high_water_mark_words = (uint32_t)uxTaskGetStackHighWaterMark(handle);

/* 🌟 MISRA 修正 20.9：預防巨集未定義的隱式擴展 */
#if defined(configGENERATE_RUN_TIME_STATS) && (configGENERATE_RUN_TIME_STATS == 1)
    out_metric->runtime_counter = ulTaskGetRunTimeCounter(handle);
#else
    out_metric->runtime_counter = 0U;
#endif

    return OS_OK;
}

uint32_t srv_os_get_sys_tick(void)
{
    return (uint32_t)xTaskGetTickCount();
}

void srv_os_delay_until(uint32_t* previous_wake_tick, uint32_t period_ms)
{
    if (previous_wake_tick != NULL)
    {
        TickType_t pt = (TickType_t)(*previous_wake_tick);
        xTaskDelayUntil(&pt, pdMS_TO_TICKS(period_ms));
        *previous_wake_tick = (uint32_t)pt;
    }
}

void srv_os_delay_ms(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

/* Hooks 實作保持不變 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName)
{
    (void)xTask;
    (void)printf("\n\n❌ [FATAL ERROR] OS Stack Overflow in task: %s\n", pcTaskName);
    while (1)
    {
    }
}

void vApplicationMallocFailedHook(void)
{
    (void)printf("\n\n❌ [FATAL ERROR] OS Malloc Failed! Out of Heap Memory.\n");
    while (1)
    {
    }
}

void vApplicationGetIdleTaskMemory(StaticTask_t** ppxIdleTaskTCBBuffer,
                                   StackType_t** ppxIdleTaskStackBuffer,
                                   uint32_t* pulIdleTaskStackSize)
{
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

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