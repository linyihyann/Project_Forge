#include "app_main.h"
#include "unity.h"

// 💡 讓 Ceedling 自動生成 HAL 與 FSM 的替身 (Mocks)
#include "mock_app_crash_dump.h"
#include "mock_app_fsm.h"
#include "mock_app_ssd1306.h"
#include "mock_hal_dio.h"
#include "mock_hal_dma.h"
#include "mock_hal_i2c.h"
#include "mock_hal_time.h"
#include "mock_observer.h"
#include "mock_ring_buffer.h"

void setUp(void)
{
    // 🌟 告訴替身：當 App Main 呼叫這些函數時，假裝有做事就好
    rb_init_Ignore();
    hal_time_start_10khz_producer_Ignore();

    // 🌟 告訴替身：當 App Main 嘗試 Dequeue 時，永遠回傳空載 (RB_EMPTY)
    rb_dequeue_IgnoreAndReturn(RB_EMPTY);

    // 🌟 [新增防禦] 忽略 I2C 與 OLED 的相關硬體呼叫，專心測 Main 的排程邏輯
    hal_i2c_init_IgnoreAndReturn(HAL_I2C_OK);
    app_ssd1306_init_IgnoreAndReturn(true);
    app_ssd1306_fill_IgnoreAndReturn(true);
    hal_i2c_bus_recovery_Ignore();
}

void tearDown(void) {}

// 測試系統初始化順序
void test_app_main_init_Should_InitializeModulesInOrder(void)
{
    observer_init_Expect();
    crash_dump_check_and_init_Expect();

    hal_dio_init_ExpectAndReturn(HAL_DIO_LED_HEARTBEAT, HAL_DIO_OK);
    app_fsm_init_Expect();
    app_fsm_process_event_ExpectAndReturn(FSM_EVENT_INIT_REQ, FSM_OK);

    // 🌟 嚴格規定它必須呼叫 DMA 初始化與啟動
    hal_uart_dma_init_ExpectAndReturn(921600, HAL_UART_OK);
    hal_uart_dma_start_rx_ExpectAndReturn(HAL_UART_OK);

    // (註：因為在 setUp 裡已經 Ignore 了 I2C 和 OLED，所以這裡不需要再寫 Expect)

    app_main_init();
}

// 測試未滿 100ms 時不應發送 TICK
void test_app_main_task_Should_NotSendTick_When_TimeUnder100ms(void)
{
    hal_time_get_ms_ExpectAndReturn(50);
    app_main_task();
}

// 測試滿 100ms 時必須發送 TICK
void test_app_main_task_Should_SendTick_When_TimeReaches100ms(void)
{
    hal_time_get_ms_ExpectAndReturn(100);
    app_fsm_process_event_ExpectAndReturn(FSM_EVENT_TICK, FSM_OK);
    app_main_task();
}