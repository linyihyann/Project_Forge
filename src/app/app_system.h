/* file: src/app/app_system.h */
#ifndef APP_SYSTEM_H
#define APP_SYSTEM_H

void app_system_init(void);
void app_task_motor_ctrl(void* arg);
void app_task_sensor_adc(void* arg);
void app_system_monitor_task(void* arg);

#endif /* APP_SYSTEM_H */