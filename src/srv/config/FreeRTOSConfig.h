/* file: FreeRTOSConfig.h */
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* ============================================================================== */
/* 🌟 ARM Cortex-M33 (RP2350) 硬體特性開關 */
/* ============================================================================== */
#define configENABLE_FPU 1
#define configENABLE_MPU 0
#define configENABLE_TRUSTZONE 0

/* ============================================================================== */
/* ☠️ [致命修正] Cortex-M 中斷優先級設定 (必須經過位移計算，否則必觸發 Hard Fault) */
/* ============================================================================== */
#define configPRIO_BITS 4                              /* RP2350 使用 4 bits 優先級 */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15     /* 系統最低優先級 (0x0F) */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5 /* OS 可管理的最高優先級 */

/* FreeRTOS 實際寫入 NVIC 暫存器的值 (需左移 4 bits) */
#define configKERNEL_INTERRUPT_PRIORITY \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* ============================================================================== */
/* [硬體與排程基礎] */
/* ============================================================================== */
#define configUSE_PREEMPTION 1
#define configUSE_TIME_SLICING 1
#define configUSE_TICKLESS_IDLE 0
#define configCPU_CLOCK_HZ 150000000
#define configTICK_RATE_HZ ((TickType_t)1000)
#define configMAX_PRIORITIES (8)
#define configMINIMAL_STACK_SIZE (256)
#define configMAX_TASK_NAME_LEN (16)

/* FreeRTOS V11 強制要求 */
#define configTICK_TYPE_WIDTH_IN_BITS TICK_TYPE_WIDTH_32_BITS
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0

/* ============================================================================== */
/* [記憶體管理與防禦機制] */
/* ============================================================================== */
#define configSUPPORT_STATIC_ALLOCATION 1
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#define configTOTAL_HEAP_SIZE ((size_t)(128 * 1024))
#define configCHECK_FOR_STACK_OVERFLOW 2
#define configUSE_MALLOC_FAILED_HOOK 1

/* ============================================================================== */
/* [IPC 跨核通訊機制] */
/* ============================================================================== */
#define configUSE_MUTEXES 1
#define configUSE_RECURSIVE_MUTEXES 1
#define configUSE_COUNTING_SEMAPHORES 1

/* ============================================================================== */
/* 🌟 [FreeRTOS API 啟用設定] 打開 Linker 需要用的功能 */
/* ============================================================================== */
#define INCLUDE_vTaskPrioritySet 1
#define INCLUDE_uxTaskPriorityGet 1
#define INCLUDE_vTaskDelete 1
#define INCLUDE_vTaskSuspend 1
#define INCLUDE_xResumeFromISR 1
#define INCLUDE_vTaskDelayUntil 1
#define INCLUDE_vTaskDelay 1
#define INCLUDE_xTaskGetSchedulerState 1
#define INCLUDE_xTaskGetCurrentTaskHandle 1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_eTaskGetState 1

/* ============================================================================== */
/* 🌟 [Software Timer] CYW43 Wi-Fi 驅動必備 */
/* ============================================================================== */
#define configUSE_TIMERS 0
#define configTIMER_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH 10
#define configTIMER_TASK_STACK_DEPTH 256

#define vPortSVCHandler isr_svcall
#define xPortPendSVHandler isr_pendsv
#define xPortSysTickHandler isr_systick

#endif /* FREERTOS_CONFIG_H */