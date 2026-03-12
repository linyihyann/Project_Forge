/*
 * ARCHITECTURE NOTE (Composition Root)
 * -----------------------------------------------
 * app_main.c 是本專案 App 層的 Composition Root。
 * 此檔案被允許 #include HAL 標頭，其唯一職責是：
 *   將 HAL 的具體實作，透過 Adapter 注入至 App 層介面。
 *
 * 規則：
 *   - 其他 app_*.c 不得直接存取 HAL
 *   - 所有硬體操作必須透過 cfg 結構的函式指標呼叫
 */

#include "app_main.h"

#include <stdio.h>  // 給 printf 報錯用

#include "app_fsm.h"
#include "app_ssd1306.h"
#include "hal_dio.h"
#include "hal_dma.h"
#include "hal_i2c.h"
#include "hal_time.h"
#include "ring_buffer.h"

// 🌟 引入觀察者與黑盒子
#include "app_crash_dump.h"
#include "observer.h"

// ==========================================
// 🌟 1. 轉接層 (Adapter Layer)
// ==========================================
static bool fsm_adapter_led_write(bool state)
{
    // 將 FSM 的 true/false 轉換為 HAL 的操作
    return (hal_dio_write(HAL_DIO_LED_HEARTBEAT, state) == HAL_DIO_OK);
}

static bool fsm_adapter_led_toggle(void)
{
    return (hal_dio_toggle(HAL_DIO_LED_HEARTBEAT) == HAL_DIO_OK);
}

static uint32_t fsm_adapter_get_ms(void)
{
    return hal_time_get_ms();
}

static bool ssd1306_adapter_i2c_tx(uint8_t dev_addr, const uint8_t* p_data, uint16_t length)
{
    return (hal_i2c_master_tx(dev_addr, p_data, length, 5000U) == HAL_I2C_OK);
}

// ==========================================
// 2. 全域變數與私有變數
// ==========================================
static uint32_t last_tick = 0U;
ring_buffer_t g_test_rb;

static const app_ssd1306_cfg_t g_ssd1306_cfg = {.i2c_tx = ssd1306_adapter_i2c_tx};
// ==========================================
// 3. 主要功能實作
// ==========================================
void app_main_init(void)
{
    last_tick = 0U;

    // ==========================================
    // 1. Observer & Crash Dump
    // ==========================================
    (void)printf("[Debug] 0. Init Observer & Crash Dump...\n");
    (void)observer_init();
    (void)crash_dump_check_and_init();

    // ==========================================
    // 2. DIO - ✅ Pico 2 W 的 LED 由 CYW43 管理
    //    hal_dio_init 會衝突，改為跳過
    // ==========================================
    (void)printf("[Debug] 1. Skip DIO (CYW43 LED managed by led_task)\n");

    // ==========================================
    // 3. FSM - LED callback 會呼叫 hal_dio，跳過
    // ==========================================
    (void)printf("[Debug] 2. Skip FSM (LED conflict with CYW43)\n");

    // ==========================================
    // 4. UART DMA
    // ==========================================
    (void)hal_uart_dma_init(921600);
    (void)hal_uart_dma_start_rx();

    // ==========================================
    // 5. Ring Buffer
    // ==========================================
    (void)printf("[Debug] 3. Init Ring Buffer...\n");
    (void)rb_init(&g_test_rb);

    // ==========================================
    // 6. I2C & SSD1306
    // ==========================================
#if 0    
    (void)printf("[Debug] 3.5 Init I2C & SSD1306...\n");
    (void)hal_i2c_init(400000U);

    if (app_ssd1306_init(&g_ssd1306_cfg) == true)
    {
        (void)printf("[INFO] SSD1306 OLED initialized successfully.\n");
    }
    else
    {
        (void)printf("[ERR] SSD1306 init failed! Check wiring.\n");
    }
#endif

    // ==========================================
    // 7. 10kHz Timer
    // ==========================================
#if 0    
    (void)printf("[Debug] 4. Start 10kHz Timer...\n");
    (void)hal_time_start_10khz_producer(&g_test_rb);

    (void)printf("10kHz Stress Test Started...\n");
#endif
}

void app_main_task(void)
{
    uint32_t now = hal_time_get_ms();

    // ==========================================
    // 🌟 定時印出存活訊息 (取代原本的 OLED 更新)
    // ==========================================
    if ((now - last_tick) >= 10000U)  // 每 10 秒執行一次
    {
        /* ❌ 封印沒有接硬體的 SSD1306 寫入動作
        static uint8_t sec_count = 0;
        sec_count++;
        if ((sec_count % 5U) == 0U) {
            static uint8_t screen_pattern = 0xFFU;
            if (app_ssd1306_fill(screen_pattern) == false) {
                (void)printf("[WARN] I2C TX Failed!\n");
                (void)hal_i2c_bus_recovery();
                (void)app_ssd1306_init(&g_ssd1306_cfg);
            }
        }
        */

        // ✅ 保留純淨的系統存活證明
        (void)printf("[System] Alive! 10kHz IPC is rock solid...\n");
        last_tick = now;
    }

    // 高頻資料處理與壓測驗證 (從 Ring Buffer 取資料)
    static uint8_t expected_val = 0;
    static bool first_read = true;
    uint8_t rx_data;

    while (rb_dequeue(&g_test_rb, &rx_data) == RB_OK)
    {
        if (first_read)
        {
            // 🌟 修正：收到第一筆資料後，我們「預期」下一筆要是它的 +1
            expected_val = (uint8_t)(rx_data + 1U);
            first_read = false;
        }
        else
        {
            if (rx_data != expected_val)
            {
                (void)printf("\n[FATAL] Data Corruption! Expected %d, got %d\n", expected_val,
                             rx_data);
                (void)observer_notify(EVENT_SYSTEM_FAULT, (void*)"Data Corruption in Ring Buffer!");

                // 發生錯誤後，重新對齊計數器，避免被連續洗版
                expected_val = rx_data;
            }
            expected_val = (uint8_t)(rx_data + 1U);
        }
    }
}