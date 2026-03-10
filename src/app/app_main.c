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
static ring_buffer_t g_test_rb;

static const app_ssd1306_cfg_t g_ssd1306_cfg = {.i2c_tx = ssd1306_adapter_i2c_tx};
// ==========================================
// 3. 主要功能實作
// ==========================================
void app_main_init(void)
{
    last_tick = 0U;

    static const app_fsm_cfg_t g_fsm_cfg = {.led_write = fsm_adapter_led_write,
                                            .led_toggle = fsm_adapter_led_toggle,
                                            .get_ms = fsm_adapter_get_ms};
    // ==========================================
    // 1. Observer & Crash Dump
    // ==========================================
    (void)printf("[Debug] 0. Init Observer & Crash Dump...\n");
    (void)observer_init();
    (void)crash_dump_check_and_init();

    // ==========================================
    // 2. DIO
    // ==========================================
    (void)printf("[Debug] 1. Init DIO...\n");
    (void)hal_dio_init(HAL_DIO_LED_HEARTBEAT);

    // ==========================================
    // 3. FSM (Dependency Injection)
    // ==========================================
    (void)printf("[Debug] 2. Init FSM with Dependency Injection...\n");
    if (app_fsm_init(&g_fsm_cfg) != FSM_OK)
    {
        (void)printf("[FATAL] FSM Init Failed! System Halted.\n");
    }
    else
    {
        (void)app_fsm_process_event(FSM_EVENT_INIT_REQ);
    }

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
    // 6. I2C & SSD1306 (Dependency Injection)
    // ==========================================
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

    // ==========================================
    // 7. 10kHz Timer
    // ==========================================
    (void)printf("[Debug] 4. Start 10kHz Timer...\n");
    (void)hal_time_start_10khz_producer(&g_test_rb);

    (void)printf("10kHz Stress Test Started...\n");
}

void app_main_task(void)
{
    uint32_t now = hal_time_get_ms();

    // 100ms 週期任務
    if ((now - last_tick) >= 100U)
    {
        // 驅動 FSM
        (void)app_fsm_process_event(FSM_EVENT_TICK);

        static uint8_t sec_count = 0;
        sec_count++;

        // 每 500ms (5 * 100ms) 更新一次螢幕，避免 I2C 佔用過多 CPU
        if ((sec_count % 5U) == 0U)
        {
            static uint8_t screen_pattern = 0xFFU;
            if (app_ssd1306_fill(screen_pattern) == false)
            {
                (void)printf("[WARN] I2C TX Failed! Triggering 9-Clock Recovery...\n");
                (void)hal_i2c_bus_recovery();

                // ✅ 復原後重新初始化，需傳入 cfg
                (void)app_ssd1306_init(&g_ssd1306_cfg);
            }
        }

        // 每 1000ms (10 * 100ms) 印出一次 Alive Log
        if (sec_count >= 10U)
        {
            // 這裡可以選擇性加入 FSM 狀態的 Log，方便除錯
            app_fsm_state_t fsm_state = app_fsm_get_state();
            (void)printf("[System] Alive! FSM State: %d. 10kHz Test running...\n", (int)fsm_state);

            sec_count = 0;
        }

        last_tick = now;
    }

    // 高頻資料處理與壓測驗證 (從 Ring Buffer 取資料)
    // 這裡的邏輯保持不變，因為它是獨立的測試模組
    static uint8_t expected_val = 0;
    static bool first_read = true;
    uint8_t rx_data;

    while (rb_dequeue(&g_test_rb, &rx_data) == RB_OK)
    {
        if (first_read)
        {
            expected_val = rx_data;
            first_read = false;
        }
        else
        {
            if (rx_data != expected_val)
            {
                (void)printf("\n[FATAL] Data Corruption! Expected %d, got %d\n", expected_val,
                             rx_data);

                // 這裡我們直接呼叫 HAL，因為這是 Main 層的緊急處理，不一定要經過 FSM
                (void)hal_dio_write(HAL_DIO_LED_HEARTBEAT, true);

                /* 🌟 MISRA Rule 11.8 Deviation: 為了配合 Observer 的通用介面 */
                // cppcheck-suppress misra-c2012-11.8
                (void)observer_notify(EVENT_SYSTEM_FAULT, (void*)"Data Corruption in Ring Buffer!");

                expected_val = rx_data;
            }
            expected_val = rx_data + 1U;
        }
    }
}