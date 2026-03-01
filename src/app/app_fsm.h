#ifndef APP_FSM_H
#define APP_FSM_H

// 狀態定義
typedef enum
{
    FSM_STATE_UNINIT = 0,
    FSM_STATE_SELF_TEST,
    FSM_STATE_RUNNING,
    FSM_STATE_FAULT
} app_fsm_state_t;

// 事件定義
typedef enum
{
    FSM_EVENT_INIT_REQ = 0,
    FSM_EVENT_TICK,     // 外部驅動的週期性 Tick 事件
    FSM_EVENT_HW_ERROR  // 外部注入的硬體錯誤事件
} app_fsm_event_t;

// 回傳狀態 (取代單純的 int)
typedef enum
{
    FSM_OK = 0,
    FSM_ERR_INVALID_EVENT,
    FSM_ERR_TIMEOUT,
    FSM_ERR_NULL_PTR
} app_fsm_status_t;

// API 宣告
void app_fsm_init(void);
app_fsm_state_t app_fsm_get_state(void);
app_fsm_status_t app_fsm_process_event(app_fsm_event_t event);

#endif  // APP_FSM_H