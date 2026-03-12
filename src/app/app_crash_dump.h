#ifndef APP_CRASH_DUMP_H
#define APP_CRASH_DUMP_H

#include <stdint.h>

#define CRASH_MAGIC_NUMBER 0xDEADBEEFU

typedef uint32_t (*app_time_provider_fn)(void);

typedef struct
{
    uint32_t magic;
    uint32_t crash_timestamp_us;
    char last_log[128];
} crash_dump_t;

extern crash_dump_t g_crash_dump;

/* =======================================================
 * API 介面
 * ======================================================= */

/* * @brief 初始化 Crash Dump 模組，並注入時間供應函數
 * @param time_fn 負責提供微秒級時間戳的函數指標 (通常會傳入 hal_time_get_us)
 */
void app_crash_dump_init(app_time_provider_fn time_fn);

void crash_dump_check_and_init(void);

#endif /* APP_CRASH_DUMP_H */