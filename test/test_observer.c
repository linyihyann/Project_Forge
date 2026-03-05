#include "observer.h"
#include "unity.h"
// ==========================================
// 🧪 測試用的 Mock Callbacks 與全域變數
// ==========================================
static uint32_t g_callback_A_called_count = 0;
static uint32_t g_callback_B_called_count = 0;

void mock_callback_A(system_event_id_t event_id, void* context)
{
    g_callback_A_called_count++;
}

void mock_callback_B(system_event_id_t event_id, void* context)
{
    g_callback_B_called_count++;
}

// ==========================================
// 🛠️ Unity 測試框架標準 Setup / Teardown
// ==========================================
void setUp(void)
{
    // 每個測試開始前，強迫將 Observer 狀態清空，保證測試獨立性！
    observer_init();

    g_callback_A_called_count = 0;
    g_callback_B_called_count = 0;
}

void tearDown(void)
{
    // 測試結束後的清理 (此模組不需要)
}

// ==========================================
// 🎯 測試案例 (Test Cases)
// ==========================================

// 測試 1：確保能正常註冊 Callback 並回傳成功 (0)
void test_observer_subscribe_should_return_success_when_normal(void)
{
    int32_t result = observer_subscribe(EVENT_SYSTEM_FAULT, mock_callback_A);
    TEST_ASSERT_EQUAL_INT32(0, result);
}

// 測試 2：【防禦性測試】傳入 NULL 必須被擋下，回傳失敗 (-1)
void test_observer_subscribe_should_reject_null_pointer(void)
{
    int32_t result = observer_subscribe(EVENT_SYSTEM_FAULT, NULL);
    TEST_ASSERT_EQUAL_INT32(-1, result);
}

// 測試 3：【邊界測試】超過 MAX_OBSERVERS 時，必須拒絕註冊並防禦 Crash
void test_observer_subscribe_should_reject_when_exceeds_max(void)
{
    // 先塞滿陣列 (MAX_OBSERVERS 定義在 header，通常是 10)
    for (int i = 0; i < MAX_OBSERVERS; i++)
    {
        int32_t res = observer_subscribe(EVENT_SYSTEM_FAULT, mock_callback_A);
        TEST_ASSERT_EQUAL_INT32(0, res);  // 前 10 個必須成功
    }

    // 第 11 個必須失敗！這證明了你的 C 語言不會發生 Buffer Overflow
    int32_t fail_res = observer_subscribe(EVENT_SYSTEM_FAULT, mock_callback_B);
    TEST_ASSERT_EQUAL_INT32(-1, fail_res);
}

// 測試 4：確保呼叫 notify 時，有註冊的人會收到，沒註冊的人不會收到
void test_observer_notify_should_trigger_correct_callbacks(void)
{
    // Callback A 訂閱 FAULT 事件
    observer_subscribe(EVENT_SYSTEM_FAULT, mock_callback_A);
    // Callback B 訂閱 UART 事件
    observer_subscribe(EVENT_UART_RX_READY, mock_callback_B);

    // 觸發 FAULT 事件
    observer_notify(EVENT_SYSTEM_FAULT, NULL);

    // 驗證：A 應該被叫 1 次，B 不該被叫到 (0 次)
    TEST_ASSERT_EQUAL_UINT32(1, g_callback_A_called_count);
    TEST_ASSERT_EQUAL_UINT32(0, g_callback_B_called_count);
}