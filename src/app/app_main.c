#include "app_main.h"

// cppcheck-suppress misra-c2012-21.6
#include <stdio.h>  // 給 printf 報錯用

#include "app_fsm.h"
#include "hal_dio.h"
#include "hal_dma.h"
#include "hal_time.h"
#include "ring_buffer.h"  // 引入資料結構

static uint32_t last_tick = 0U;

// 🌟 宣告我們的主角：Ring Buffer
static ring_buffer_t g_test_rb;

void app_main_init(void)
{
    last_tick = 0U;

    // cppcheck-suppress misra-c2012-17.7
    // cppcheck-suppress misra-c2012-21.6
    printf("[Debug] 1. Init DIO...\n");
    (void)hal_dio_init(HAL_DIO_LED_HEARTBEAT);

    // cppcheck-suppress misra-c2012-17.7
    // cppcheck-suppress misra-c2012-21.6
    printf("[Debug] 2. Init FSM...\n");
    app_fsm_init();
    (void)app_fsm_process_event(FSM_EVENT_INIT_REQ);

    (void)hal_uart_dma_init(921600);
    (void)hal_uart_dma_start_rx();

    // cppcheck-suppress misra-c2012-17.7
    // cppcheck-suppress misra-c2012-21.6
    printf("[Debug] 3. Init Ring Buffer...\n");
    (void)rb_init(&g_test_rb);

    // cppcheck-suppress misra-c2012-17.7
    // cppcheck-suppress misra-c2012-21.6
    printf("[Debug] 4. Start 10kHz Timer...\n");
    (void)hal_time_start_10khz_producer(&g_test_rb);

    // cppcheck-suppress misra-c2012-17.7
    // cppcheck-suppress misra-c2012-21.6
    printf("10kHz Stress Test Started...\n");
}

void app_main_task(void)
{
    uint32_t now = hal_time_get_ms();

    if ((now - last_tick) >= 100U)
    {
        (void)app_fsm_process_event(FSM_EVENT_TICK);

        // 🌟 補上這段心跳包：每 10 次 100ms (即 1 秒) 印出一次存活證明
        static uint8_t sec_count = 0;
        sec_count++;
        if (sec_count >= 10U)
        {
            // cppcheck-suppress misra-c2012-17.7
            printf("[System] Alive! 10kHz Stress Test is running...\n");
            sec_count = 0;
        }

        last_tick = now;
    }

    // ==========================================
    // 🛡️ 壓測專用：Consumer 驗證邏輯
    // ==========================================
    static uint8_t expected_val = 0;
    static bool first_read = true;
    uint8_t rx_data;

    // while 迴圈：只要裡面有資料，就全速抽乾它
    while (rb_dequeue(&g_test_rb, &rx_data) == RB_OK)
    {
        if (first_read)
        {
            expected_val = rx_data;  // 第一次先對齊基準點
            first_read = false;
        }

        // 致命防禦檢查：數字如果不連續，代表 Lock-free 失敗或爆滿了！
        if (rx_data != expected_val)
        {
            // cppcheck-suppress misra-c2012-17.7
            // cppcheck-suppress misra-c2012-21.6
            printf("\n[FATAL] Data Corruption! Expected %d, got %d\n", expected_val, rx_data);

            // 將板子上的 LED 長亮表示錯誤
            (void)hal_dio_write(HAL_DIO_LED_HEARTBEAT, true);

            expected_val = rx_data;  // 重新對齊，繼續觀察
        }

        expected_val++;
    }

    hal_uart_packet_t packet;
    if (hal_uart_dma_get_ready_packet(&packet) == HAL_UART_OK)
    {
        hal_uart_dma_release_packet();
    }
}