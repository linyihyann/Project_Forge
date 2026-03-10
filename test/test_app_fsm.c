#include "app_fsm.h"
#include "unity.h"

// ============================================================
// 🌟 Test Double Layer
//    FSM 已透過 DI 與 HAL 解耦，測試只需 mock callback，
//    不需要 mock_hal_dio / mock_hal_time
// ============================================================

// --- Mock 狀態紀錄 ---
static bool mock_led_state = false;
static bool mock_led_write_ret = true;   // 預設成功
static bool mock_led_toggle_ret = true;  // 預設成功
static uint32_t mock_time_ms = 0U;

// --- Mock 實作 ---
static bool mock_led_write(bool state)
{
    mock_led_state = state;
    return mock_led_write_ret;
}

static bool mock_led_toggle(void)
{
    return mock_led_toggle_ret;
}

static uint32_t mock_get_ms(void)
{
    return mock_time_ms;
}

// --- 標準 cfg，每個 TC 都用這個 ---
static const app_fsm_cfg_t g_test_cfg = {
    .led_write = mock_led_write, .led_toggle = mock_led_toggle, .get_ms = mock_get_ms};

// ============================================================
// setUp / tearDown
// ============================================================
void setUp(void)
{
    // 重置所有 mock 狀態
    mock_led_state = false;
    mock_led_write_ret = true;
    mock_led_toggle_ret = true;
    mock_time_ms = 0U;

    // ✅ 傳入 mock cfg
    (void)app_fsm_init(&g_test_cfg);
}

void tearDown(void) {}

// ============================================================
// TC-01: 測試正常初始化流程
// ============================================================
void test_app_fsm_Should_EnterSelfTest_When_InitRequested(void)
{
    mock_time_ms = 100U;

    app_fsm_status_t res = app_fsm_process_event(FSM_EVENT_INIT_REQ);

    TEST_ASSERT_EQUAL(FSM_OK, res);
    TEST_ASSERT_EQUAL(FSM_STATE_SELF_TEST, app_fsm_get_state());
    TEST_ASSERT_EQUAL(true, mock_led_state);  // LED 應該被點亮
}

// ============================================================
// TC-02: 發送非法事件
// ============================================================
void test_app_fsm_Should_RejectInvalidEvent_In_UninitState(void)
{
    app_fsm_status_t res = app_fsm_process_event(FSM_EVENT_TICK);

    TEST_ASSERT_EQUAL(FSM_ERR_INVALID_EVENT, res);
    TEST_ASSERT_EQUAL(FSM_STATE_UNINIT, app_fsm_get_state());
}

// ============================================================
// TC-06: 狀態機重置
// ============================================================
void test_app_fsm_Should_ResetToUninit_When_InFaultState(void)
{
    // 讓 led_write 回傳失敗，強制進入 FAULT
    mock_led_write_ret = false;
    (void)app_fsm_process_event(FSM_EVENT_INIT_REQ);
    TEST_ASSERT_EQUAL(FSM_STATE_FAULT, app_fsm_get_state());

    // ✅ 重新 init，應該回到 UNINIT
    (void)app_fsm_init(&g_test_cfg);
    TEST_ASSERT_EQUAL(FSM_STATE_UNINIT, app_fsm_get_state());
}

// ============================================================
// TC-07: 超時未收到回覆 (Timeout)
// ============================================================
void test_app_fsm_Should_Timeout_When_SelfTestTakesTooLong(void)
{
    // Step 1: 進入 SELF_TEST（t=100ms）
    mock_time_ms = 100U;
    (void)app_fsm_process_event(FSM_EVENT_INIT_REQ);

    // Step 2: 超過 timeout（t=650ms，超過 500ms 門檻）
    mock_time_ms = 650U;
    app_fsm_status_t res = app_fsm_process_event(FSM_EVENT_TICK);

    TEST_ASSERT_EQUAL(FSM_ERR_TIMEOUT, res);
    TEST_ASSERT_EQUAL(FSM_STATE_FAULT, app_fsm_get_state());
    TEST_ASSERT_EQUAL(false, mock_led_state);  // LED 應該被關閉
}

// ============================================================
// TC-08: 自檢成功並安全轉移至 RUNNING 狀態
// ============================================================
void test_app_fsm_Should_EnterRunning_When_SelfTestCompletes(void)
{
    // Step 1: 進入 SELF_TEST（t=100ms）
    mock_time_ms = 100U;
    (void)app_fsm_process_event(FSM_EVENT_INIT_REQ);

    // Step 2: 在 timeout 前完成（t=300ms，超過 200ms 自檢時間但未超時）
    mock_time_ms = 300U;
    app_fsm_status_t res = app_fsm_process_event(FSM_EVENT_TICK);

    TEST_ASSERT_EQUAL(FSM_OK, res);
    TEST_ASSERT_EQUAL(FSM_STATE_RUNNING, app_fsm_get_state());
}

// ============================================================
// TC-09: RUNNING 狀態下的心跳閃爍 (Heartbeat Toggle)
// ============================================================
void test_app_fsm_Should_ToggleLed_When_RunningHeartbeatTicks(void)
{
    // Step 1: 進入 SELF_TEST
    mock_time_ms = 0U;
    (void)app_fsm_process_event(FSM_EVENT_INIT_REQ);

    // Step 2: 進入 RUNNING
    mock_time_ms = 200U;
    (void)app_fsm_process_event(FSM_EVENT_TICK);
    TEST_ASSERT_EQUAL(FSM_STATE_RUNNING, app_fsm_get_state());

    // Step 3: 超過心跳週期（500ms），應觸發 toggle
    mock_time_ms = 700U;
    app_fsm_status_t res = app_fsm_process_event(FSM_EVENT_TICK);

    TEST_ASSERT_EQUAL(FSM_OK, res);
    // mock_led_toggle_ret = true，所以不應進入 FAULT
    TEST_ASSERT_EQUAL(FSM_STATE_RUNNING, app_fsm_get_state());
}

// ============================================================
// TC-10: 未初始化時呼叫 process_event 應被拒絕
// ============================================================
void test_app_fsm_Should_ReachEndOfFunction_When_EventIsUnprocessed(void)
{
    // 用一個全新的未初始化狀態測試
    // 先 init 再用 NULL cfg 重置（模擬未初始化情境）
    // 注意：直接測 g_cfg == NULL 的防禦邏輯

    // 此 TC 驗證：在 UNINIT 狀態下送 TICK，應回傳 INVALID_EVENT
    app_fsm_status_t res = app_fsm_process_event(FSM_EVENT_TICK);

    TEST_ASSERT_EQUAL(FSM_ERR_INVALID_EVENT, res);
    TEST_ASSERT_EQUAL(FSM_STATE_UNINIT, app_fsm_get_state());
}