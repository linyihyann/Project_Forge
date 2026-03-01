#include "hal_dio.h"
#include "pico/cyw43_arch.h"

hal_dio_status_t hal_dio_init(hal_dio_pin_t pin)
{
    if (pin == HAL_DIO_LED_HEARTBEAT)
    {
        if (cyw43_arch_init() == 0)
        {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            return HAL_DIO_OK;
        }
        else
        {
            // 🚨 將硬體初始化失敗的狀況，誠實地往上報給 App 層！
            return HAL_DIO_ERR_HW_INIT_FAIL;
        }
    }
    return HAL_DIO_ERR_INVALID_PIN;
}

hal_dio_status_t hal_dio_write(hal_dio_pin_t pin, bool is_high)
{
    if (pin == HAL_DIO_LED_HEARTBEAT)
    {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, is_high);
        return HAL_DIO_OK;
    }
    return HAL_DIO_ERR_INVALID_PIN;
}

hal_dio_status_t hal_dio_toggle(hal_dio_pin_t pin)
{
    if (pin == HAL_DIO_LED_HEARTBEAT)
    {
        static bool state = true;
        state = !state;
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, state);
        return HAL_DIO_OK;
    }
    return HAL_DIO_ERR_INVALID_PIN;
}