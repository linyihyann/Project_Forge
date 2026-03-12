#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "app_crash_dump.h"
#include "app_fs_stress.h"
#include "app_main.h"
#include "app_system.h"
#include "hal_dio.h"
#include "hal_dma.h"
#include "hal_i2c.h"
#include "hal_time.h"
#include "hardware/irq.h"
#include "hardware/timer.h"
#include "hardware/watchdog.h"
#include "observer.h"
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "ring_buffer.h"
#include "srv_os.h"
#include "task.h"
#include "tusb.h"

void isr_hardfault(void);
static void core1_hard_rt_loop(void);

#define MAIN_LOOP_DELAY_MS 10U
#define WDT_TIMEOUT_MS 2000U

static volatile bool g_core0_ready = false;
/* ============================================================================
 * Hard Fault 偵測器
 * ============================================================================ */
void isr_hardfault(void)
{
    while (1)
    {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        uint32_t t = time_us_32();
        // 加上括號與 U 後綴，讓優先級與型態絕對明確
        while (((time_us_32() - t) < 100000U))
        {
            tight_loop_contents();
        }
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        t = time_us_32();
        // 加上括號與 U 後綴，讓優先級與型態絕對明確
        while (((time_us_32() - t) < 100000U))
        {
            tight_loop_contents();
        }
    }
}

static void trigger_hardware_reset(void)
{
    (void)watchdog_enable(1, 1);
    while (1)
    {
    }
}

// ============================================================================
// 🚀 [Core 1] 裸機硬即時任務 (Hard Real-Time, 絕對零抖動)
// ============================================================================
static void core1_hard_rt_loop(void)
{
    while (!g_core0_ready)
    {
        tight_loop_contents();
    }

    // 設定 10kHz (100us) 的控制週期
    const uint64_t PERIOD_US = 100;
    uint64_t next_wake_time = time_us_64() + PERIOD_US;

    uint8_t tx_data = 0;  // 測試用的流水號 (0~255 循環)

    while (1)
    {
        // 1. 精準等待：這比任何 OS Delay 都還要精準，保證微秒級零抖動
        while (time_us_64() < next_wake_time)
        {
            tight_loop_contents();
        }
        next_wake_time += PERIOD_US;

        // 2. 塞入 Lock-Free Ring Buffer
        // 🌟 絕對安全：不涉及任何 OS 鎖，只使用原子操作 (Atomic Operations)
        if (rb_enqueue(&g_test_rb, tx_data) == RB_OK)  // ✅ 正確：直接傳遞數值
        {
            tx_data++;
        }
        else
        {
            // 如果 Buffer 滿了，代表 Core 0 (FreeRTOS) 處理太慢
            // 實務上這裡可以累加一個 dropped_count 來做效能監控
        }
    }
}

/* ============================================================================
 * LED 心跳任務 (獨立運作，不依賴 USB)
 * ============================================================================ */
static void led_task(void* arg)
{
    (void)arg;
    bool led_state = false;
    while (1)
    {
        led_state = !led_state;
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// ============================================================================
// 🐢 [Core 0] FreeRTOS 業務主任務 (Soft Real-Time)
// ============================================================================
static void app_main_os_task(void* arg)
{
    (void)arg;

    uint32_t loop_counter = 0;
    (void)printf("🐢 [Core 0] OS 業務任務已啟動...\n");

    // 🌟 亮綠燈！通知 Core 1：OS 已經準備好接資料了，開火！
    g_core0_ready = true;

    while (1)
    {
        (void)watchdog_update();  // 餵狗
        (void)app_main_task();    // 🌟 每 10ms 快速抽乾一次 Ring Buffer (約 100 筆資料)

        loop_counter++;
        srv_os_delay_ms(MAIN_LOOP_DELAY_MS);

        // 🌟 每 100 個 Loop (10ms * 100 = 1000ms) 印一次 Log，維持畫面乾淨
        if ((loop_counter % 100U) == 0U)
        {
            (void)printf("🔄 [Core 0] 系統正常運行中... (Loop %" PRIu32 ")\n", loop_counter / 100U);
        }
    }
}

static void init_task(void* arg)
{
    (void)arg;

    while (!tud_cdc_connected())
    {
        (void)vTaskDelay(pdMS_TO_TICKS(100));
    }

    (void)vTaskDelay(pdMS_TO_TICKS(500));
    (void)printf("✅ USB Connected!\r\n");

    (void)printf("[Init] Step 1: app_system_init...\r\n");
    (void)app_system_init();
    (void)printf("[Init] Step 1: 完成!\r\n");

    (void)printf("[Init] Step 2: Watchdog...\r\n");
    (void)watchdog_enable(WDT_TIMEOUT_MS, 1);

    (void)printf("[Init] Step 3: Core 1...\r\n");
    (void)multicore_launch_core1(core1_hard_rt_loop);

    (void)printf("[Init] Step 4: AppMain 任務...\r\n");
    (void)srv_os_create_task(app_main_os_task, "AppMain", 2048, NULL, 1);

    (void)printf("[Init] 全部完成！\r\n");
    (void)vTaskDelete(NULL);
}

int main(void)
{
    stdio_init_all();

    /* ✅ 立刻設定，不等進任務 */
    irq_set_priority(USBCTRL_IRQ, 0x40);

    cyw43_arch_init();

    xTaskCreate(led_task, "LED", 512, NULL, 1, NULL);
    xTaskCreate(init_task, "Init", 2048, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1)
    {
    }
    return 0;
}