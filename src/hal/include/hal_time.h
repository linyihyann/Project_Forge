#ifndef HAL_TIME_H
#define HAL_TIME_H
#include <stdint.h>

// 取得系統當前毫秒數 (供 App 層 FSM 計算 Timeout 使用)
uint32_t hal_time_get_ms(void);
void hal_time_start_10khz_producer(void* rb_ptr);

#endif  // HAL_TIME_H