#ifndef HAL_DIO_H
#define HAL_DIO_H
#include <stdbool.h>

typedef enum
{
    HAL_DIO_LED_HEARTBEAT = 0,
    HAL_DIO_MAX_PINS
} hal_dio_pin_t;

// 定義標準錯誤回傳碼
typedef enum
{
    HAL_DIO_OK = 0,
    HAL_DIO_ERR_INVALID_PIN,
    HAL_DIO_ERR_HW_INIT_FAIL  // 硬體初始化失敗
} hal_dio_status_t;

// 介面全面改用狀態碼回傳
hal_dio_status_t hal_dio_init(hal_dio_pin_t pin);
hal_dio_status_t hal_dio_write(hal_dio_pin_t pin, bool is_high);
hal_dio_status_t hal_dio_toggle(hal_dio_pin_t pin);

#endif  // HAL_DIO_H