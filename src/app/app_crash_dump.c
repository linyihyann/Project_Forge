#include "app_crash_dump.h"

#include <inttypes.h>
#include <string.h>

// 引入觀察者介面，實現模組解耦
#include "observer.h"

// cppcheck-suppress misra-c2012-21.6
#include <stdio.h>

__attribute__((section(".uninitialized_data"))) crash_dump_t g_crash_dump;

// =======================================================
// 🌟 核心業務邏輯：將字串安全寫入黑盒子
// =======================================================
void crash_dump_save_log(const char* log_msg)
{
    if (log_msg != NULL)
    {
        (void)strncpy(g_crash_dump.last_log, log_msg, sizeof(g_crash_dump.last_log) - 1U);
        g_crash_dump.last_log[sizeof(g_crash_dump.last_log) - 1U] = '\0';

        g_crash_dump.crash_timestamp_us = hal_time_get_us();

        g_crash_dump.magic = CRASH_MAGIC_NUMBER;
    }
}

// =======================================================
// 🌟 觀察者中介層：接收廣播並轉發給業務邏輯
// =======================================================
static void app_crash_dump_fault_handler(system_event_id_t event_id, void* context)
{
    if ((event_id == EVENT_SYSTEM_FAULT) && (context != NULL))
    {
        /* 🌟 MISRA Rule 11.5 Deviation: Observer 模式必須透過 void* 傳遞通用 context */
        // cppcheck-suppress misra-c2012-11.5
        crash_dump_save_log((const char*)context);
    }
}

// =======================================================
// 初始化與檢查邏輯
// =======================================================
void crash_dump_check_and_init(void)
{
    if (g_crash_dump.magic == CRASH_MAGIC_NUMBER)
    {
        (void)printf("\r\n🚨 [FATAL EXCEPTION DETECTED] 🚨\r\n");
        (void)printf("⚠️ 偵測到 Watchdog / HardFault 異常重啟！\r\n");
        (void)printf("⚠️ 上次死機前 Boot Time: %" PRIu32 " us\r\n", g_crash_dump.crash_timestamp_us);
        (void)printf("⚠️ 死機前最後遺言: %s\r\n", g_crash_dump.last_log);
        (void)printf("========================================\r\n");

        g_crash_dump.magic = 0U;  // 清除 Flag
    }
    else
    {
        (void)printf("[INFO] 系統正常冷啟動 (Cold Boot) 或 RAM 已斷電。\r\n");
    }

    // 向系統廣播電台訂閱「致命錯誤」事件
    (void)observer_subscribe(EVENT_SYSTEM_FAULT, app_crash_dump_fault_handler);
}