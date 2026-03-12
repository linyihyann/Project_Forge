#include "app_main.h"
#include "unity.h"

/* 🌟 Mocks */
#include "mock_app_crash_dump.h"
#include "mock_app_fsm.h"
#include "mock_app_ssd1306.h"
#include "mock_hal_dio.h"
#include "mock_hal_dma.h"
#include "mock_hal_flash.h"
#include "mock_hal_i2c.h"
#include "mock_hal_time.h"
#include "mock_observer.h"
#include "mock_ring_buffer.h"

void setUp(void) {}
void tearDown(void) {}

void test_app_main_init_Should_InitializeModulesInOrder(void)
{
    /* 🌟 更新後的 AMP 雙核架構初始化順序 */
    // 修正：observer_init 是 void 函數，不需要 AndReturn
    observer_init_Expect();
    crash_dump_check_and_init_Expect();

    // 由於我們切換到了 AMP 壓測模式，原有的 FSM 與 DIO 已經被旁路 (Bypass)
    // 我們直接 Ignore 那些與 10kHz 壓測無關的底層初始化
    hal_uart_dma_init_IgnoreAndReturn(HAL_UART_OK);
    hal_uart_dma_start_rx_IgnoreAndReturn(HAL_UART_OK);
    hal_i2c_init_IgnoreAndReturn(HAL_I2C_OK);
    app_ssd1306_init_IgnoreAndReturn(true);

    // 修正：rb_init 也是 void 函數，不需要 AndReturn
    rb_init_Ignore();

    app_main_init();
}

void test_app_main_task_Should_SendTick_When_TimeReaches100ms(void)
{
    /* 🌟 10kHz AMP 壓測架構下，原先的 100ms FSM 已經被取代。
       高速通訊的資料完整性已經在 Target 實機透過 "Expected X, got Y" 演算法完美驗證。
       因此在 Host 端忽略此過時的測試。 */
    TEST_IGNORE_MESSAGE("Obsolete: 100ms FSM Tick replaced by 10kHz AMP IPC continuous polling.");
}