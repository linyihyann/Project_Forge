#include "app_system.h"

#include "app_crash_dump.h"
#include "app_fsm.h"
#include "app_main.h"
#include "app_ssd1306.h"

void app_system_init(void)
{
    (void)app_main_init();
}
