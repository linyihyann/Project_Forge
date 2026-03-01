#include "app_main.h"

#include "app_fsm.h"
#include "hal_dio.h"
#include "hal_time.h"

void app_main_init(void)
{
    // 💡 修正 MISRA 17.7：加上 (void)
    (void)hal_dio_init(HAL_DIO_LED_HEARTBEAT);

    app_fsm_init();

    (void)app_fsm_process_event(FSM_EVENT_INIT_REQ);
}

void app_main_task(void)
{
    // 💡 修正 MISRA 10.4：靜態變數初始化也應使用 U
    static uint32_t last_tick = 0U;
    uint32_t now = hal_time_get_ms();

    // 💡 修正 MISRA 10.4 & 12.1：加上括號明確優先級，並將 100 改為 100U
    if ((now - last_tick) >= 100U)
    {
        // 💡 修正 MISRA 17.7：加上 (void)
        (void)app_fsm_process_event(FSM_EVENT_TICK);
        last_tick = now;
    }
}