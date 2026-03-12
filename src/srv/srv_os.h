/* file: src/srv/srv_os.h */
#ifndef SRV_OS_H
#define SRV_OS_H

#include <stdbool.h>
#include <stdint.h>

/* 定義任務函數的指標型別 */
typedef void (*srv_os_task_func_t)(void* arg);

/* OS 初始化與啟動 */
void srv_os_init(void);
void srv_os_start_scheduler(void);

/* 任務建立與控制 */
bool srv_os_create_task(srv_os_task_func_t func, const char* name, uint32_t stack_words, void* arg,
                        uint8_t priority);

/* 系統延遲 (不會卡死 CPU，會交出執行權) */
void srv_os_delay_ms(uint32_t ms);

#endif /* SRV_OS_H */