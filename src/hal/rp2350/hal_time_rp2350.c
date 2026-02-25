#include "hal_time.h"
#include "pico/time.h"

// 🚨 這裡絕對不要 include 任何跟 LED 或 DIO 有關的標頭檔

uint32_t hal_time_get_ms(void)
{
    // 取得開機以來的毫秒數
    return to_ms_since_boot(get_absolute_time());
}

void hal_time_delay_ms(uint32_t ms)
{
    // 呼叫底層的延遲函數
    sleep_ms(ms);
}