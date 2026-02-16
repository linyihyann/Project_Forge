#include "app_main.h"
#include "hal_time.h"
#include "hal_dio.h"

void app_main_init(void) {
    // 呼叫這行時，底層的自檢機制就會讓燈亮起來
    hal_dio_init(HAL_DIO_LED_HEARTBEAT); 
}

void app_main_task(void) {
    // 🌟 先等半秒鐘，讓你可以看到開機自檢的那道光
    hal_time_delay_ms(500); 
    
    // 然後開始切換
    hal_dio_toggle(HAL_DIO_LED_HEARTBEAT); 
}