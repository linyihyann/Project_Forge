#include "app_main.h"

#include "app_fsm.h"
#include "hal_dio.h"
#include "hal_time.h"

void app_main_init(void)
{
    hal_dio_init(HAL_DIO_LED_HEARTBEAT);
    app_fsm_init();
    // 💡 啟動狀態機進入 Self-Test
    app_fsm_process_event(FSM_EVENT_INIT_REQ);
}

void app_main_task(void)
{
    static uint32_t last_tick = 0;
    uint32_t now = hal_time_get_ms();

    // 🌟 每 100ms 餵一次 Tick，維持狀態機運作
    if (now - last_tick >= 100)
    {
        app_fsm_process_event(FSM_EVENT_TICK);
        last_tick = now;
    }
}