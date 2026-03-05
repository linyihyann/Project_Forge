#include "app_main.h"

// cppcheck-suppress misra-c2012-21.6
#include <stdio.h>  // 給 printf 報錯用

#include "app_fsm.h"
#include "hal_dio.h"
#include "hal_dma.h"
#include "hal_time.h"
#include "ring_buffer.h"  // 引入資料結構

// 🌟 引入觀察者與黑盒子
#include "app_crash_dump.h"
#include "observer.h"

static uint32_t last_tick = 0U;
static ring_buffer_t g_test_rb;

void app_main_init(void)
{
    last_tick = 0U;

    (void)printf("[Debug] 0. Init Observer & Crash Dump...\n");
    (void)observer_init();              // 🌟 1. 開啟廣播電台
    (void)crash_dump_check_and_init();  // 🌟 2. 檢查黑盒子，並向電台註冊訂閱

    (void)printf("[Debug] 1. Init DIO...\n");
    (void)hal_dio_init(HAL_DIO_LED_HEARTBEAT);

    (void)printf("[Debug] 2. Init FSM...\n");
    app_fsm_init();
    (void)app_fsm_process_event(FSM_EVENT_INIT_REQ);

    (void)hal_uart_dma_init(921600);
    (void)hal_uart_dma_start_rx();

    (void)printf("[Debug] 3. Init Ring Buffer...\n");
    (void)rb_init(&g_test_rb);

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
        (void)app_fsm_process_event(FSM_EVENT_TICK);

        static uint8_t sec_count = 0;
        sec_count++;
        if (sec_count >= 10U)
        {
            (void)printf("[System] Alive! 10kHz Stress Test is running...\n");
            sec_count = 0;
        }

        last_tick = now;
    }

    // 高頻資料處理與壓測驗證
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
                (void)hal_dio_write(HAL_DIO_LED_HEARTBEAT, true);

                /* 🌟 MISRA Rule 11.8 Deviation: 為了配合 Observer 的通用介面，暫時移除字串的 const
                 * 屬性 */
                // cppcheck-suppress misra-c2012-11.8
                (void)observer_notify(EVENT_SYSTEM_FAULT, (void*)"Data Corruption in Ring Buffer!");

                expected_val = rx_data;
            }
            expected_val = rx_data + 1U;
        }
    }
}