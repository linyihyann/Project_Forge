#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_crash_dump.h"
#include "app_fs_stress.h"
#include "app_main.h"
#include "app_system.h"
#include "hal_time.h"
#include "hardware/watchdog.h"
#include "pico/stdlib.h"

// ==========================================
// ⚙️ 系統防禦與故障注入參數設定 (消滅魔術數字)
// ==========================================
#define MAIN_LOOP_DELAY_MS 500U  // 主迴圈每次休眠時間 (毫秒)
#define WDT_TIMEOUT_MS 2000U     // 看門狗咬人極限時間 (毫秒)

// ⚠️ 確保故障注入的時間點，必定早於看門狗觸發時間
#define FAULT_INJECTION_TIME_MS 5000U  // 預期在開機後 1.5 秒注入致命故障
#define FAULT_INJECTION_LOOP_COUNT (FAULT_INJECTION_TIME_MS / MAIN_LOOP_DELAY_MS)

static void trigger_hardware_reset(void)
{
    // 利用 Pico SDK 的 Watchdog 強制觸發重啟
    (void)watchdog_enable(1, 1);
    while (1)
    {
    }
}

int main(void)
{
    (void)stdio_init_all();

    int wait_count = 0;
    while ((stdio_usb_connected() == false) && (wait_count < 30))
    {
        (void)sleep_ms(100);
        wait_count++;
    }

    if (stdio_usb_connected() == true)
    {
        (void)sleep_ms(500);
        (void)printf("\n\n====================================\n");
        (void)printf("✅ USB Serial Connected Successfully!\n");
    }

    app_fs_stress_execute(hal_time_get_us, trigger_hardware_reset);

    // 🌟 2. 死後驗屍機制：檢查並印出上次是否為 Watchdog 當機重啟？
    // crash_dump_check_and_init();

    // =========================================================
    // 🌟 3. 精準 Profiling：量測純軟體初始化時間
    // =========================================================
    uint32_t t1_us = hal_time_get_us();

    (void)app_system_init();

    uint32_t t2_us = hal_time_get_us();
    uint32_t real_init_time_us = t2_us - t1_us;

    (void)printf("🚀 [PERF] App Initialization Time: %" PRIu32 " us\n", real_init_time_us);

    // =========================================================
    // 🌟 4. 啟動 Watchdog 防禦
    // =========================================================
    (void)watchdog_enable(WDT_TIMEOUT_MS, 1);

    uint32_t loop_counter = 0;

    // 5. 進入無窮迴圈 (Super Loop)
    while (1)
    {
        (void)watchdog_update();

        (void)app_main_task();

        loop_counter++;
        (void)sleep_ms(MAIN_LOOP_DELAY_MS);
        (void)printf("🔄 系統正常運行中... (Loop %" PRIu32 ")\n", loop_counter);

        if (loop_counter == FAULT_INJECTION_LOOP_COUNT)
        {
            (void)printf("\n😈 [FAULT INJECTION] 模擬 App 層死鎖或 HardFault...\n");

            // 寫下遺言到 .uninitialized_data 黑盒子區塊
            (void)crash_dump_save_log("FATAL: System deadlocked! WDT will trigger soon.");

            while (1)
            {
                // 系統死鎖，卡在這裡不餵狗。
                // 等待時間達到 WDT_TIMEOUT_MS 後，RP2350 會被強制 Reset！
            }
        }
    }

    return 0;
}