#pragma once

// 系統應用層初始化
void app_main_init(void);

// 系統應用層主任務 (未來上 RTOS 前的 Super Loop 進入點)
void app_main_task(void);