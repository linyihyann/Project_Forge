#ifndef HAL_DIO_H
#define HAL_DIO_H

#include <stdbool.h>

// 定義車規級的抽象腳位
typedef enum {
    HAL_DIO_LED_HEARTBEAT,
    HAL_DIO_MAX_PINS
} hal_dio_pin_t;

// 介面宣告
void hal_dio_init(hal_dio_pin_t pin);
void hal_dio_write(hal_dio_pin_t pin, bool is_high);
void hal_dio_toggle(hal_dio_pin_t pin);

#endif // HAL_DIO_H