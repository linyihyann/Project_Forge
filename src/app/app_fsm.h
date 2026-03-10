#ifndef APP_FSM_H
#define APP_FSM_H

#include <stdbool.h>
#include <stdint.h>

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
    FSM_EVENT_TICK,
    FSM_EVENT_HW_ERROR
} app_fsm_event_t;

// 回傳狀態
typedef enum
{
    FSM_OK = 0,
    FSM_ERR_INVALID_EVENT,
    FSM_ERR_TIMEOUT,
    FSM_ERR_NULL_PTR,
    FSM_ERR_HW_ACCESS_FAIL
} app_fsm_status_t;

// 🌟 新增：介面定義 (依賴注入)
// FSM 不需要知道底層是怎麼開燈的，它只需要一個能控制 LED 的介面
// 回傳 true 代表成功，false 代表失敗
typedef bool (*app_fsm_gpio_write_cb_t)(bool state);
typedef bool (*app_fsm_gpio_toggle_cb_t)(void);
typedef uint32_t (*app_fsm_get_time_cb_t)(void);

typedef struct
{
    app_fsm_gpio_write_cb_t led_write;
    app_fsm_gpio_toggle_cb_t led_toggle;
    app_fsm_get_time_cb_t get_ms;
} app_fsm_cfg_t;

// API 宣告
// Init 現在需要傳入設定檔指標
app_fsm_status_t app_fsm_init(const app_fsm_cfg_t* cfg);
app_fsm_state_t app_fsm_get_state(void);
app_fsm_status_t app_fsm_process_event(app_fsm_event_t event);

#endif  // APP_FSM_H