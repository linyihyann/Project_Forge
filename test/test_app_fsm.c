#include "app_fsm.h"
#include "mock_hal_dio.h"
#include "mock_hal_time.h"
#include "unity.h"

void setUp(void)
{
    app_fsm_init();
}
void tearDown(void) {}

// TC-01: 測試正常初始化流程
void test_app_fsm_Should_EnterSelfTest_When_InitRequested(void)
{
    hal_dio_write_ExpectAndReturn(HAL_DIO_LED_HEARTBEAT, true, HAL_DIO_OK);
    hal_time_get_ms_ExpectAndReturn(100);
    app_fsm_status_t res = app_fsm_process_event(FSM_EVENT_INIT_REQ);
    TEST_ASSERT_EQUAL(FSM_OK, res);
    TEST_ASSERT_EQUAL(FSM_STATE_SELF_TEST, app_fsm_get_state());
}

// TC-02: 發送非法事件
void test_app_fsm_Should_RejectInvalidEvent_In_UninitState(void)
{
    app_fsm_status_t res = app_fsm_process_event(FSM_EVENT_TICK);
    TEST_ASSERT_EQUAL(FSM_ERR_INVALID_EVENT, res);
    TEST_ASSERT_EQUAL(FSM_STATE_UNINIT, app_fsm_get_state());
}

// TC-07: 超時未收到回覆 (Timeout)
void test_app_fsm_Should_Timeout_When_SelfTestTakesTooLong(void)
{
    hal_dio_write_ExpectAndReturn(HAL_DIO_LED_HEARTBEAT, true, HAL_DIO_OK);
    hal_time_get_ms_ExpectAndReturn(100);
    app_fsm_process_event(FSM_EVENT_INIT_REQ);

    hal_time_get_ms_ExpectAndReturn(650);
    hal_dio_write_ExpectAndReturn(HAL_DIO_LED_HEARTBEAT, false, HAL_DIO_OK);
    app_fsm_status_t res = app_fsm_process_event(FSM_EVENT_TICK);
    TEST_ASSERT_EQUAL(FSM_ERR_TIMEOUT, res);
    TEST_ASSERT_EQUAL(FSM_STATE_FAULT, app_fsm_get_state());
}

// TC-06: 狀態機重置
void test_app_fsm_Should_ResetToUninit_When_InFaultState(void)
{
    hal_dio_write_ExpectAndReturn(HAL_DIO_LED_HEARTBEAT, true, HAL_DIO_ERR_HW_INIT_FAIL);
    app_fsm_process_event(FSM_EVENT_INIT_REQ);
    app_fsm_init();
    TEST_ASSERT_EQUAL(FSM_STATE_UNINIT, app_fsm_get_state());
}

// TC-08: 自檢成功並安全轉移至 RUNNING 狀態
void test_app_fsm_Should_EnterRunning_When_SelfTestCompletes(void)
{
    hal_dio_write_ExpectAndReturn(HAL_DIO_LED_HEARTBEAT, true, HAL_DIO_OK);
    hal_time_get_ms_ExpectAndReturn(100);
    app_fsm_process_event(FSM_EVENT_INIT_REQ);

    hal_time_get_ms_ExpectAndReturn(300);
    app_fsm_status_t res = app_fsm_process_event(FSM_EVENT_TICK);
    TEST_ASSERT_EQUAL(FSM_OK, res);
    TEST_ASSERT_EQUAL(FSM_STATE_RUNNING, app_fsm_get_state());
}

// TC-09: RUNNING 狀態下的心跳閃爍 (Heartbeat Toggle)
void test_app_fsm_Should_ToggleLed_When_RunningHeartbeatTicks(void)
{
    hal_dio_write_ExpectAndReturn(HAL_DIO_LED_HEARTBEAT, true, HAL_DIO_OK);
    hal_time_get_ms_ExpectAndReturn(0);
    app_fsm_process_event(FSM_EVENT_INIT_REQ);

    hal_time_get_ms_ExpectAndReturn(200);
    app_fsm_process_event(FSM_EVENT_TICK);

    hal_time_get_ms_ExpectAndReturn(700);
    hal_dio_toggle_ExpectAndReturn(HAL_DIO_LED_HEARTBEAT, HAL_DIO_OK);
    app_fsm_status_t res = app_fsm_process_event(FSM_EVENT_TICK);
    TEST_ASSERT_EQUAL(FSM_OK, res);
}

// TC-10: 測試在 RUNNING 狀態下，收到未預期的事件 (例如突然又叫它 Init)
void test_app_fsm_Should_RejectInvalidEvent_In_RunningState(void)
{
    // 快速推入 RUNNING
    hal_dio_write_ExpectAndReturn(HAL_DIO_LED_HEARTBEAT, true, HAL_DIO_OK);
    hal_time_get_ms_ExpectAndReturn(0);
    app_fsm_process_event(FSM_EVENT_INIT_REQ);
    hal_time_get_ms_ExpectAndReturn(200);
    app_fsm_process_event(FSM_EVENT_TICK);

    // 在 RUNNING 狀態下亂發 INIT_REQ
    app_fsm_status_t res = app_fsm_process_event(FSM_EVENT_INIT_REQ);
    TEST_ASSERT_EQUAL(FSM_ERR_INVALID_EVENT, res);
}

// TC-11: 測試在 RUNNING 狀態下，突發硬體損壞 (HW ERROR) 必須安全降級
void test_app_fsm_Should_EnterFault_When_HwErrorInRunning(void)
{
    // 快速推入 RUNNING
    hal_dio_write_ExpectAndReturn(HAL_DIO_LED_HEARTBEAT, true, HAL_DIO_OK);
    hal_time_get_ms_ExpectAndReturn(0);
    app_fsm_process_event(FSM_EVENT_INIT_REQ);
    hal_time_get_ms_ExpectAndReturn(200);
    app_fsm_process_event(FSM_EVENT_TICK);

    // 模擬底層驅動發出求救訊號 (HW_ERROR)
    hal_dio_write_ExpectAndReturn(HAL_DIO_LED_HEARTBEAT, false, HAL_DIO_OK);  // 預期會強制關燈
    app_fsm_status_t res = app_fsm_process_event(FSM_EVENT_HW_ERROR);

    TEST_ASSERT_EQUAL(FSM_OK, res);
    TEST_ASSERT_EQUAL(FSM_STATE_FAULT, app_fsm_get_state());
}

// TC-12: 測試在 FAULT 狀態下，系統必須進入假死，拒絕一切 Tick
void test_app_fsm_Should_DoNothing_When_InFaultState(void)
{
    // 故意讓初始化失敗，進入 FAULT
    hal_dio_write_ExpectAndReturn(HAL_DIO_LED_HEARTBEAT, true, HAL_DIO_ERR_HW_INIT_FAIL);
    app_fsm_process_event(FSM_EVENT_INIT_REQ);

    // 在 FAULT 狀態下繼續餵 Tick
    app_fsm_status_t res = app_fsm_process_event(FSM_EVENT_TICK);
    TEST_ASSERT_EQUAL(FSM_ERR_INVALID_EVENT, res);
    TEST_ASSERT_EQUAL(FSM_STATE_FAULT, app_fsm_get_state());
}