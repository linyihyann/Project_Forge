#include "app_fsm.h"

#include "hal_dio.h"
#include "hal_time.h"

// 💡 修正 MISRA 10.4：加上 U，明確宣告為無號數
#define FSM_SELF_TEST_TIMEOUT_MS 500U
#define FSM_SELF_TEST_DURATION_MS 200U
#define FSM_HEARTBEAT_PERIOD_MS 500U

static app_fsm_state_t current_state = FSM_STATE_UNINIT;
static uint32_t state_start_time = 0;

void app_fsm_init(void)
{
    current_state = FSM_STATE_UNINIT;
    state_start_time = 0;
}

app_fsm_state_t app_fsm_get_state(void)
{
    return current_state;
}

app_fsm_status_t app_fsm_process_event(app_fsm_event_t event)
{
    // 🌟 移除這裡的全域獲取，改在需要的 Case 裡面精準獲取

    switch (current_state)
    {
        case FSM_STATE_UNINIT:
            if (event == FSM_EVENT_INIT_REQ)
            {
                // 先點亮 LED (對齊測試的 Expect 順序)
                if (hal_dio_write(HAL_DIO_LED_HEARTBEAT, true) != HAL_DIO_OK)
                {
                    current_state = FSM_STATE_FAULT;
                    return FSM_ERR_INVALID_EVENT;
                }
                // 燈亮成功後，才紀錄起始時間
                state_start_time = hal_time_get_ms();
                current_state = FSM_STATE_SELF_TEST;
                return FSM_OK;
            }
            return FSM_ERR_INVALID_EVENT;

        case FSM_STATE_SELF_TEST:
            if (event == FSM_EVENT_TICK)
            {
                // 只在 TICK 事件發生時才去取時間
                uint32_t current_time = hal_time_get_ms();
                uint32_t elapsed_time = current_time - state_start_time;

                if (elapsed_time > FSM_SELF_TEST_TIMEOUT_MS)
                {
                    (void)hal_dio_write(HAL_DIO_LED_HEARTBEAT, false);
                    current_state = FSM_STATE_FAULT;
                    return FSM_ERR_TIMEOUT;
                }
                else if (elapsed_time >= FSM_SELF_TEST_DURATION_MS)
                {
                    current_state = FSM_STATE_RUNNING;
                    state_start_time = current_time;
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
                uint32_t current_time = hal_time_get_ms();
                uint32_t elapsed_time = current_time - state_start_time;

                if (elapsed_time >= FSM_HEARTBEAT_PERIOD_MS)
                {
                    (void)hal_dio_toggle(HAL_DIO_LED_HEARTBEAT);
                    state_start_time = current_time;
                }
                return FSM_OK;
            }
            if (event == FSM_EVENT_HW_ERROR)
            {
                (void)hal_dio_write(HAL_DIO_LED_HEARTBEAT, false);
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
void app_fsm_test_hook_set_state(app_fsm_state_t state)
{
    current_state = state;
}
#endif