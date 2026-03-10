#include "app_crash_dump.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "observer.h"

/* 將全域變數放在未初始化 RAM 區段，實作死後驗屍黑盒子 */
__attribute__((section(".uninitialized_data"))) crash_dump_t g_crash_dump;

/* =======================================================
 * 🌟 [架構解耦] 儲存注入進來的時間函數指標
 * ======================================================= */
static app_time_provider_fn get_time_us_cb = NULL;

/* =======================================================
 * 🌟 依賴注入 API
 * ======================================================= */
void app_crash_dump_init(app_time_provider_fn time_fn)
{
    if (time_fn != NULL)
    {
        get_time_us_cb = time_fn;
    }
}

/* =======================================================
 * 核心業務邏輯：將字串安全寫入黑盒子
 * ======================================================= */
void crash_dump_save_log(const char* log_msg)
{
    if (log_msg != NULL)
    {
        /* MISRA 防禦：確保字串不溢位 */
        (void)strncpy(g_crash_dump.last_log, log_msg, sizeof(g_crash_dump.last_log) - 1U);
        g_crash_dump.last_log[sizeof(g_crash_dump.last_log) - 1U] = '\0';

        /* 🌟 使用外部注入的函數取得時間戳 (若尚未注入則防呆給 0) */
        g_crash_dump.crash_timestamp_us = (get_time_us_cb != NULL) ? get_time_us_cb() : 0U;

        g_crash_dump.magic = CRASH_MAGIC_NUMBER;
    }
}

/* =======================================================
 * 觀察者中介層：接收廣播並轉發給業務邏輯
 * ======================================================= */
static void app_crash_dump_fault_handler(system_event_id_t event_id, void* context)
{
    if ((event_id == EVENT_SYSTEM_FAULT) && (context != NULL))
    {
        /* MISRA Rule 11.5 Deviation: Observer 模式必須透過 void* 傳遞通用 context */
        // cppcheck-suppress misra-c2012-11.5
        crash_dump_save_log((const char*)context);
    }
}

/* =======================================================
 * 初始化與死後驗屍檢查邏輯
 * ======================================================= */
void crash_dump_check_and_init(void)
{
    if (g_crash_dump.magic == CRASH_MAGIC_NUMBER)
    {
        (void)printf("\r\n🚨 [FATAL EXCEPTION DETECTED] 🚨\r\n");
        (void)printf("⚠️ 偵測到 Watchdog / HardFault 異常重啟！\r\n");
        (void)printf("⚠️ 上次死機前 Boot Time: %" PRIu32 " us\r\n", g_crash_dump.crash_timestamp_us);
        (void)printf("⚠️ 最後遺言: %s\r\n\r\n", g_crash_dump.last_log);
    }
    else
    {
        (void)printf("\r\n✅ [SYSTEM BOOT] 系統正常冷開機，無異常當機紀錄。\r\n");
    }

    /* 檢查完畢後，清除 Magic Number 避免下次正常重啟誤判 */
    g_crash_dump.magic = 0U;
    g_crash_dump.crash_timestamp_us = 0U;
    (void)memset(g_crash_dump.last_log, 0, sizeof(g_crash_dump.last_log));

    /* 註冊 Observer，訂閱嚴重錯誤事件 */
    (void)observer_subscribe(EVENT_SYSTEM_FAULT, app_crash_dump_fault_handler);
}