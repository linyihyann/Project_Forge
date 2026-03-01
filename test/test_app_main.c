#include "app_main.h"
#include "unity.h"

// 💡 讓 Ceedling 自動生成 HAL 與 FSM 的替身 (Mocks)
#include "mock_app_fsm.h"
#include "mock_hal_dio.h"
#include "mock_hal_time.h"

void setUp(void) {}
void tearDown(void) {}

// 測試系統初始化順序
void test_app_main_init_Should_InitializeModulesInOrder(void)
{
    hal_dio_init_ExpectAndReturn(HAL_DIO_LED_HEARTBEAT, HAL_DIO_OK);
    app_fsm_init_Expect();
    app_fsm_process_event_ExpectAndReturn(FSM_EVENT_INIT_REQ, FSM_OK);

    app_main_init();
}

// 測試未滿 100ms 時不應發送 TICK
void test_app_main_task_Should_NotSendTick_When_TimeUnder100ms(void)
{
    hal_time_get_ms_ExpectAndReturn(50);
    // 未設定 fsm_process_event_Expect，若被呼叫 CMock 會報錯！
    app_main_task();
}

// 測試滿 100ms 時必須發送 TICK
void test_app_main_task_Should_SendTick_When_TimeReaches100ms(void)
{
    hal_time_get_ms_ExpectAndReturn(100);
    app_fsm_process_event_ExpectAndReturn(FSM_EVENT_TICK, FSM_OK);

    app_main_task();
}