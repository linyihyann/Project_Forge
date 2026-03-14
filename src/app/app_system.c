/* file: src/app/app_system.c */
#include "app_system.h"

#include <inttypes.h>
#include <stdio.h>

#include "app_main.h"
#include "srv_os.h"

/* ============================================================================ */
/* 🌟 Unit Test Hook: 讓 Ceedling 測試時不會卡死在無窮迴圈，且不增加產線 ROM 負擔 */
/* ============================================================================ */
#ifdef TEST
#define FOR_EVER       \
    int test_loop = 1; \
    test_loop == 1;    \
    test_loop = 0
#else
#define FOR_EVER \
    ;            \
    ;
#endif

void app_system_init(void)
{
    (void)app_main_init();
}

/* ============================================================================ */
/* 🌟 純 CPU 算力消耗器 (模擬真實演算法的指令消耗，不受 OS 搶佔干擾) */
/* ============================================================================ */
static void dummy_workload_cpu_cycles(uint32_t target_loops)
{
    /* 使用 volatile 強迫編譯器不要把這個無意義的迴圈優化掉 */
    for (volatile uint32_t i = 0U; i < target_loops; i++)
    {
        __asm volatile("nop");
    }
}

/* ============================================================================ */
/* 🏎️ 高頻控制任務 (依據 RMS 理論配置) */
/* ============================================================================ */

/* Task: 馬達控制 (週期 1ms) */
void app_task_motor_ctrl(void* arg)
{
    (void)arg;

    /* 取得乾淨的 OSAL 時間基準 */
    uint32_t last_wake_time = srv_os_get_sys_tick();
    const uint32_t MOTOR_LOOP_COUNT = 3000U;

    for (FOR_EVER)
    {
        dummy_workload_cpu_cycles(MOTOR_LOOP_COUNT);

        /* 絕對時間延遲，確保硬即時週期 */
        srv_os_delay_until(&last_wake_time, 1U);
    }
}

/* Task: 感測器採集 (週期 5ms, 負載約 20%) */
void app_task_sensor_adc(void* arg)
{
    (void)arg;

    /* 取得乾淨的 OSAL 時間基準 */
    uint32_t last_wake_time = srv_os_get_sys_tick();
    const uint32_t SENSOR_LOOP_COUNT = 15000U;

    for (FOR_EVER)
    {
        dummy_workload_cpu_cycles(SENSOR_LOOP_COUNT);

        /* 絕對時間延遲，確保硬即時週期 */
        srv_os_delay_until(&last_wake_time, 5U);
    }
}

/* ============================================================================ */
/* 👁️ 系統健康度監控任務 (System Monitor) */
/* ============================================================================ */
void app_system_monitor_task(void* arg)
{
    (void)arg;
    os_task_metric_t metric;

    for (FOR_EVER)
    {
        (void)printf("\n=== 📊 [System Monitor] Task Health Report ===\n");

        if (srv_os_get_task_metric(TASK_ID_MOTOR_CTRL, &metric) == OS_OK)
        {
            (void)printf("[MotorCtrl] Stack Margin: %" PRIu32 " Words, CPU Runtime: %" PRIu32
                         " us\n",
                         metric.stack_high_water_mark_words, metric.runtime_counter);
        }

        if (srv_os_get_task_metric(TASK_ID_SENSOR_ADC, &metric) == OS_OK)
        {
            (void)printf("[SensorADC] Stack Margin: %" PRIu32 " Words, CPU Runtime: %" PRIu32
                         " us\n",
                         metric.stack_high_water_mark_words, metric.runtime_counter);
        }

        if (srv_os_get_task_metric(TASK_ID_APP_MAIN, &metric) == OS_OK)
        {
            (void)printf("[AppMain]   Stack Margin: %" PRIu32 " Words\n",
                         metric.stack_high_water_mark_words);
        }
        (void)printf("==============================================\n");

        /* Monitor 不需要硬即時，使用普通的 delay_ms 即可交出 CPU 執行權 */
        srv_os_delay_ms(1000U);
    }
}