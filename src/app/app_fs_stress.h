#ifndef APP_FS_STRESS_H
#define APP_FS_STRESS_H

#include <stdint.h>

typedef uint32_t (*app_time_cb_t)(void);
typedef void (*app_system_reset_cb_t)(void);

void app_fs_stress_execute(app_time_cb_t get_time_cb, app_system_reset_cb_t reset_cb);

#endif /* APP_FS_STRESS_H */