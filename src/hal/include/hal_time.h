#ifndef HAL_TIME_H
#define HAL_TIME_H
#include <stdint.h>

uint32_t hal_time_get_ms(void);
uint32_t hal_time_get_us(void);  // 👈 確保這行有加上去並存檔！
void hal_time_start_10khz_producer(void* rb_ptr);

#endif  // HAL_TIME_H