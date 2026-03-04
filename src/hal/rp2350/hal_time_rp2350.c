#include "hal_time.h"
#include "hardware/sync.h"
#include "pico/time.h"
#include "ring_buffer.h"  // 引入資料結構介面

// 🌟 1. 實作真實的 ARM 硬體記憶體屏障
void hal_atomic_dmb(void)
{
    __dmb();
}

uint32_t hal_time_get_ms(void)
{
    return to_ms_since_boot(get_absolute_time());
}

// ==========================================
// 🚨 壓測專用：10kHz 硬體中斷 (Producer)
// ==========================================
static uint8_t isr_counter = 0;
static struct repeating_timer timer_10khz;

// Timer ISR Callback
static bool timer_10khz_isr_callback(struct repeating_timer* t)
{
    ring_buffer_t* rb = (ring_buffer_t*)t->user_data;

    // 🌟 只有當成功塞入 Buffer 時，數字才允許遞增
    if (rb_enqueue(rb, isr_counter) == RB_OK)
    {
        isr_counter++;
    }
    // 如果 Buffer 滿了，數字就不動，等 Main Loop 來抽乾

    return true;
}

// 供 App 層呼叫來啟動壓測
void hal_time_start_10khz_producer(void* rb_ptr)
{
    // -100us 代表精確的 10kHz 週期
    add_repeating_timer_us(-100, timer_10khz_isr_callback, rb_ptr, &timer_10khz);
}