#include "app_crash_dump.h"

#include <inttypes.h>  // 🌟 引入標準整數格式化巨集
#include <string.h>

// cppcheck-suppress misra-c2012-21.6
#include <stdio.h>

__attribute__((section(".uninitialized_data"))) crash_dump_t g_crash_dump;

void crash_dump_check_and_init(void)
{
    if (g_crash_dump.magic == CRASH_MAGIC_NUMBER)
    {
        // 🌟 MISRA Rule 17.7: 明確標示 (void) 以忽略回傳值
        (void)printf("\r\n🚨 [FATAL EXCEPTION DETECTED] 🚨\r\n");
        (void)printf("⚠️ 偵測到 Watchdog / HardFault 異常重啟！\r\n");

        // 🌟 修正 printf arg type: 使用 PRIu32 確保跨平台 uint32_t 格式正確
        (void)printf("⚠️ 上次死機前 Boot Time: %" PRIu32 " us\r\n", g_crash_dump.boot_time_us);

        (void)printf("⚠️ 死機前最後遺言: %s\r\n", g_crash_dump.last_log);
        (void)printf("========================================\r\n");

        g_crash_dump.magic = 0U;  // 🌟 MISRA Rule 7.2: 加上 U
    }
    else
    {
        (void)printf("[INFO] 系統正常冷啟動 (Cold Boot) 或 RAM 已斷電。\r\n");
    }

    // 🌟 MISRA Rule 17.7: 忽略 memset 回傳值
    (void)memset(&g_crash_dump, 0, sizeof(crash_dump_t));
    g_crash_dump.magic = CRASH_MAGIC_NUMBER;
}

void crash_dump_save_log(const char* msg)
{
    // 🌟 MISRA Rule 17.7 (忽略回傳) & Rule 10.4 (使用 1U 確保型別一致)
    (void)strncpy(g_crash_dump.last_log, msg, sizeof(g_crash_dump.last_log) - 1U);
    g_crash_dump.last_log[sizeof(g_crash_dump.last_log) - 1U] = '\0';
}