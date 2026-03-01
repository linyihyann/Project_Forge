#include "hal_time.h"
#include "pico/time.h"

uint32_t hal_time_get_ms(void)
{
    return to_ms_since_boot(get_absolute_time());
}

// 如果底層驅動自己需要 sleep，請寫在私有函數，不要 expose 到 .h