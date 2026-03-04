#include "app_main.h"
#include "unity.h"

// 💡 讓 Ceedling 自動生成 HAL 與 FSM 的替身 (Mocks)
#include "mock_app_fsm.h"
#include "mock_hal_dio.h"
#include "mock_hal_time.h"
// 🌟 加上這行：告訴 CMock 幫我們產生 UART DMA 的假函數
#include "mock_hal_dma.h"

void setUp(void) {}
void tearDown(void) {}

// 測試系統初始化順序
void test_app_main_init_Should_InitializeModulesInOrder(void)
{
    hal_dio_init_ExpectAndReturn(HAL_DIO_LED_HEARTBEAT, HAL_DIO_OK);
    app_fsm_init_Expect();
    app_fsm_process_event_ExpectAndReturn(FSM_EVENT_INIT_REQ, FSM_OK);

    // 🌟 新增這兩行：嚴格規定它必須呼叫 DMA 初始化與啟動
    hal_uart_dma_init_ExpectAndReturn(921600, HAL_UART_OK);
    hal_uart_dma_start_rx_ExpectAndReturn(HAL_UART_OK);

    app_main_init();
}

// 測試未滿 100ms 時不應發送 TICK
void test_app_main_task_Should_NotSendTick_When_TimeUnder100ms(void)
{
    hal_time_get_ms_ExpectAndReturn(50);

    // 🌟 模擬 DMA 輪詢：告訴它「目前沒有新資料」
    hal_uart_dma_get_ready_packet_ExpectAnyArgsAndReturn(HAL_UART_ERR_NO_DATA);

    app_main_task();
}

// 測試滿 100ms 時必須發送 TICK
void test_app_main_task_Should_SendTick_When_TimeReaches100ms(void)
{
    hal_time_get_ms_ExpectAndReturn(100);
    app_fsm_process_event_ExpectAndReturn(FSM_EVENT_TICK, FSM_OK);

    // 🌟 模擬 DMA 輪詢：告訴它「目前沒有新資料」
    hal_uart_dma_get_ready_packet_ExpectAnyArgsAndReturn(HAL_UART_ERR_NO_DATA);

    app_main_task();
}