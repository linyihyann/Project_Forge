#ifndef APP_MAIN_H
#define APP_MAIN_H

#pragma once

#include "ring_buffer.h"

// 系統應用層初始化
void app_main_init(void);

// 系統應用層主任務 (未來上 RTOS 前的 Super Loop 進入點)
void app_main_task(void);
// 🌟 為了符合 MISRA-C 8.4，將 Test Hook 宣告於標頭檔中

#ifdef TEST
void app_main_test_hook_reset_tick(void);
#endif

extern ring_buffer_t g_test_rb;

#endif  // 結束原有的 include guard