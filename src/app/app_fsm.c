#include "app_fsm.h"

#include <stddef.h>  // for NULL

// 💡 修正 MISRA 10.4：加上 U，明確宣告為無號數
#define FSM_SELF_TEST_TIMEOUT_MS 500U
#define FSM_SELF_TEST_DURATION_MS 200U
#define FSM_HEARTBEAT_PERIOD_MS 500U

static app_fsm_state_t current_state = FSM_STATE_UNINIT;
static uint32_t state_start_time = 0U;

// 🌟 儲存外部注入的介面
static const app_fsm_cfg_t* g_fsm_cfg = NULL;

app_fsm_status_t app_fsm_init(const app_fsm_cfg_t* cfg)
{
    // MISRA Rule 17.7: 檢查指標是否為空
    if (cfg == NULL)
    {
        return FSM_ERR_NULL_PTR;
    }

    // 檢查內部的函數指標是否為空 (防禦性編程)
    if ((cfg->led_write == NULL) || (cfg->led_toggle == NULL) || (cfg->get_ms == NULL))
    {
        return FSM_ERR_NULL_PTR;
    }

    g_fsm_cfg = cfg;
    current_state = FSM_STATE_UNINIT;
    state_start_time = 0U;

    return FSM_OK;
}

app_fsm_state_t app_fsm_get_state(void)
{
    return current_state;
}

app_fsm_status_t app_fsm_process_event(app_fsm_event_t event)
{
    // MISRA 防禦：如果未初始化或 Config 是空的，拒絕執行
    if (g_fsm_cfg == NULL)
    {
        return FSM_ERR_NULL_PTR;
    }

    switch (current_state)
    {
        case FSM_STATE_UNINIT:
            if (event == FSM_EVENT_INIT_REQ)
            {
                // 🌟 使用 callback，不再直接呼叫 hal_dio_write
                if (g_fsm_cfg->led_write(true) == false)
                {
                    current_state = FSM_STATE_FAULT;
                    return FSM_ERR_HW_ACCESS_FAIL;
                }

                // 燈亮成功後，才紀錄起始時間
                state_start_time = g_fsm_cfg->get_ms();
                current_state = FSM_STATE_SELF_TEST;
                return FSM_OK;
            }
            return FSM_ERR_INVALID_EVENT;

        case FSM_STATE_SELF_TEST:
            if (event == FSM_EVENT_TICK)
            {
                uint32_t current_time = g_fsm_cfg->get_ms();
                uint32_t elapsed_time = current_time - state_start_time;

                if (elapsed_time > FSM_SELF_TEST_TIMEOUT_MS)
                {
                    (void)g_fsm_cfg->led_write(false);
                    current_state = FSM_STATE_FAULT;
                    return FSM_ERR_TIMEOUT;
                }
                else if (elapsed_time >= FSM_SELF_TEST_DURATION_MS)
                {
                    current_state = FSM_STATE_RUNNING;
                    state_start_time = current_time;  // 重置時間給下一個狀態用
                    return FSM_OK;
                }
                else
                {
                    // MISRA 15.7: 防禦性編程，等待測試時間到達
                }
                return FSM_OK;
            }
            return FSM_ERR_INVALID_EVENT;

        case FSM_STATE_RUNNING:
            if (event == FSM_EVENT_TICK)
            {
                uint32_t current_time = g_fsm_cfg->get_ms();
                uint32_t elapsed_time = current_time - state_start_time;

                if (elapsed_time >= FSM_HEARTBEAT_PERIOD_MS)
                {
                    (void)g_fsm_cfg->led_toggle();
                    state_start_time = current_time;
                }
                return FSM_OK;
            }
            if (event == FSM_EVENT_HW_ERROR)
            {
                (void)g_fsm_cfg->led_write(false);
                current_state = FSM_STATE_FAULT;
                return FSM_OK;
            }
            return FSM_ERR_INVALID_EVENT;

        case FSM_STATE_FAULT:
            break;

        default:
            current_state = FSM_STATE_FAULT;
            break;
    }
    return FSM_ERR_INVALID_EVENT;
}

#ifdef TEST
void app_fsm_test_hook_set_state(app_fsm_state_t state);
void app_fsm_test_hook_set_state(app_fsm_state_t state)
{
    current_state = state;
}
#endif