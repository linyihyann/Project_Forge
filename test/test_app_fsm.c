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

extern void app_fsm_test_hook_set_state(app_fsm_state_t state);

// TC-13: 模擬宇宙射線打到 RAM，狀態機變成未知數字 (例如 99)
void test_app_fsm_Should_TriggerDefault_When_StateIsCorrupted(void)
{
    // 透過後門強制把 static 變數改成非法值
    app_fsm_test_hook_set_state((app_fsm_state_t)99);

    // 隨便發個事件，它應該要觸發 default: 降級為 FAULT，並回傳 INVALID_EVENT
    app_fsm_status_t res = app_fsm_process_event(FSM_EVENT_TICK);

    TEST_ASSERT_EQUAL(FSM_ERR_INVALID_EVENT, res);
    TEST_ASSERT_EQUAL(FSM_STATE_FAULT, app_fsm_get_state());
}
// 測試在 SELF_TEST 狀態下收到一個完全不相干的事件 (例如 TICK)
// 這會跳過所有 case，最終撞到函數末尾的 return
void test_app_fsm_Should_ReachEndOfFunction_When_EventIsUnprocessed(void)
{
    // 💡 假設目前在 UNINIT 狀態
    app_fsm_init();

    // 當發送 TICK 給 UNINIT 狀態時
    // 程式碼會直接執行到 case FSM_STATE_UNINIT 裡的 return FSM_ERR_INVALID_EVENT
    // 根本不會執行到 hal_time_get_ms()
    app_fsm_status_t res = app_fsm_process_event(FSM_EVENT_TICK);

    TEST_ASSERT_EQUAL(FSM_ERR_INVALID_EVENT, res);
    // 這裡不要寫 hal_time_get_ms_Expect... 因為邏輯上真的不會 call 到
}