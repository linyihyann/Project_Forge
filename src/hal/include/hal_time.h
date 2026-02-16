#ifndef HAL_TIME_H
#define HAL_TIME_H

#include <stdint.h>

// 取得系統當前毫秒數
uint32_t hal_time_get_ms(void);

// 🚨 補上這行：讓 App 層可以看到這個「死等」延遲函數
void hal_time_delay_ms(uint32_t ms);

#endif // HAL_TIME_H