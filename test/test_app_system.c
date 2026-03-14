/* file: test/test_app_system.c */
#include "app_system.h"
#include "mock_app_main.h"
#include "mock_srv_os.h"
#include "unity.h"

void setUp(void) {}
void tearDown(void) {}

void test_app_system_monitor_task_Should_ProcessHealthyMetrics(void)
{
    /* 1. 準備假數據 (Arrange) - 補上 Main Task 的假數據 */
    os_task_metric_t fake_motor_metric = {.stack_high_water_mark_words = 200U,
                                          .runtime_counter = 50000U};
    os_task_metric_t fake_sensor_metric = {.stack_high_water_mark_words = 150U,
                                           .runtime_counter = 40000U};
    os_task_metric_t fake_main_metric = {.stack_high_water_mark_words = 1900U,
                                         .runtime_counter = 1000U};

    /* 2. 期望的函數呼叫順序 (Expect) - 嚴格按照程式碼的順序 */

    /* 期待呼叫 1: 詢問 Motor */
    srv_os_get_task_metric_ExpectAndReturn(TASK_ID_MOTOR_CTRL, NULL, OS_OK);
    srv_os_get_task_metric_IgnoreArg_out_metric();
    srv_os_get_task_metric_ReturnThruPtr_out_metric(&fake_motor_metric);

    /* 期待呼叫 2: 詢問 Sensor */
    srv_os_get_task_metric_ExpectAndReturn(TASK_ID_SENSOR_ADC, NULL, OS_OK);
    srv_os_get_task_metric_IgnoreArg_out_metric();
    srv_os_get_task_metric_ReturnThruPtr_out_metric(&fake_sensor_metric);

    /* 🌟 期待呼叫 3: 詢問 AppMain (把這塊補齊！) */
    srv_os_get_task_metric_ExpectAndReturn(TASK_ID_APP_MAIN, NULL, OS_OK);
    srv_os_get_task_metric_IgnoreArg_out_metric();
    srv_os_get_task_metric_ReturnThruPtr_out_metric(&fake_main_metric);

    /* 期待呼叫 4: 進入 Delay */
    srv_os_delay_ms_Expect(1000U);

    /* 3. 執行測試 (Act) */
    app_system_monitor_task(NULL);
}