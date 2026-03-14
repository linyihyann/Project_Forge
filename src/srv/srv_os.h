/* file: src/srv/srv_os.h */
#ifndef SRV_OS_H
#define SRV_OS_H

#include <stdbool.h>
#include <stdint.h>

/* MISRA C: 嚴格定義狀態碼 */
typedef enum
{
    OS_OK = 0,
    OS_ERR_PARAM = 1,
    OS_ERR_NOT_FOUND = 2
} os_status_t;

/* 定義任務 ID (涵蓋所有會用到的 Task) */
typedef enum
{
    TASK_ID_SYS_MONITOR = 0,
    TASK_ID_MOTOR_CTRL,
    TASK_ID_SENSOR_ADC,
    TASK_ID_APP_MAIN,
    TASK_ID_LED,
    TASK_ID_INIT,
    TASK_ID_MAX_COUNT
} os_task_id_t;

/* 定義系統健康度指標結構 */
typedef struct
{
    uint32_t stack_high_water_mark_words;
    uint32_t runtime_counter;
} os_task_metric_t;

typedef void (*srv_os_task_func_t)(void* arg);

void srv_os_init(void);
void srv_os_start_scheduler(void);

/* 加入 ID 參數的新版任務建立介面 */
bool srv_os_create_task(os_task_id_t id, srv_os_task_func_t func, const char* name,
                        uint32_t stack_words, void* arg, uint8_t priority);

/* 取得任務健康度介面 */
os_status_t srv_os_get_task_metric(os_task_id_t id, os_task_metric_t* out_metric);

void srv_os_delay_ms(uint32_t ms);

/* 完全不依賴 FreeRTOS 型別的介面 */
uint32_t srv_os_get_sys_tick(void);
void srv_os_delay_until(uint32_t* previous_wake_tick, uint32_t ms);

#endif /* SRV_OS_H */