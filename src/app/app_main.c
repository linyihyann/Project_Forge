#include "app_main.h"

#include "app_fsm.h"
#include "hal_dio.h"
#include "hal_dma.h"
#include "hal_time.h"

// 🌟 Tier-1 做法：提升為檔案級靜態變數，方便統一管理生命週期
static uint32_t last_tick = 0U;

void app_main_init(void)
{
    last_tick = 0U;

    (void)hal_dio_init(HAL_DIO_LED_HEARTBEAT);
    app_fsm_init();
    (void)app_fsm_process_event(FSM_EVENT_INIT_REQ);

    // 🌟 加上 (void) 告訴 MISRA 分析器：我們已知並刻意忽略此回傳值
    (void)hal_uart_dma_init(921600);
    (void)hal_uart_dma_start_rx();
}

void app_main_task(void)
{
    uint32_t now = hal_time_get_ms();

    if ((now - last_tick) >= 100U)
    {
        (void)app_fsm_process_event(FSM_EVENT_TICK);
        last_tick = now;
    }

    hal_uart_packet_t packet;
    if (hal_uart_dma_get_ready_packet(&packet) == HAL_UART_OK)
    {
        // 先不做處理，馬上釋放
        hal_uart_dma_release_packet();
    }
}

// 🌟 專為 TDD 設計的測試後門 (Test Hook)
#ifdef TEST
void app_main_test_hook_reset_tick(void)
{
    last_tick = 0U;
}
#endif
