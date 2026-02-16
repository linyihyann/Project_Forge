#include "hal_dio.h"
#include "pico/cyw43_arch.h"

void hal_dio_init(hal_dio_pin_t pin) {
    if (pin == HAL_DIO_LED_HEARTBEAT) {
        // 🚨 拔除 stdio_init_all()，排除任何 USB/UART 的干擾
        
        // 初始化無線晶片
        if (cyw43_arch_init() == 0) {
            // 🌟 開機自檢 (POST)：只要初始化成功，馬上強制亮綠燈！
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        }
    }
}

void hal_dio_write(hal_dio_pin_t pin, bool is_high) {
    if (pin == HAL_DIO_LED_HEARTBEAT) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, is_high);
    }
}

void hal_dio_toggle(hal_dio_pin_t pin) {
    if (pin == HAL_DIO_LED_HEARTBEAT) {
        // 因為 init 已經強制亮燈了，所以這裡從 true 開始反轉
        static bool state = true; 
        state = !state;
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, state);
    }
}