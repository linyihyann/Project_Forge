#include <stdint.h>
#include <stdio.h>

#include "app_crash_dump.h"
#include "app_main.h"
#include "hal_time.h"
#include "hardware/watchdog.h"
#include "pico/stdlib.h"

// ==========================================
// ⚙️ 系統防禦與故障注入參數設定 (消滅魔術數字)
// ==========================================
#define MAIN_LOOP_DELAY_MS 500U  // 主迴圈每次休眠時間 (毫秒)
#define WDT_TIMEOUT_MS 2000U     // 看門狗咬人極限時間 (毫秒)

// ⚠️ 確保故障注入的時間點，必定早於看門狗觸發時間
#define FAULT_INJECTION_TIME_MS 1500U  // 預期在開機後 1.5 秒注入致命故障
#define FAULT_INJECTION_LOOP_COUNT (FAULT_INJECTION_TIME_MS / MAIN_LOOP_DELAY_MS)

int main(void)
{
    // 1. 初始化所有標準輸出 (包含 USB)
    stdio_init_all();

    // =========================================================
    // 🌟 終極防禦：卡住開機流程，直到你用電腦連上 USB 為止！
    // =========================================================
    while (!stdio_usb_connected())
    {
        sleep_ms(100);
    }

    // 連線成功後，稍微緩衝 0.5 秒讓終端機準備好
    sleep_ms(500);

    (void)printf("\n\n====================================\n");
    (void)printf("✅ USB Serial Connected Successfully!\n");
    (void)printf("====================================\n\n");

    // 🌟 2. 死後驗屍機制：檢查並印出上次是否為 Watchdog 當機重啟？
    crash_dump_check_and_init();

    // =========================================================
    // 🌟 3. 精準 Profiling：量測純軟體初始化時間
    // =========================================================
    uint32_t t1_us = hal_time_get_us();

    // 開始系統初始化 (這裡面會啟動你的 10kHz 壓測等周邊)
    app_main_init();

    uint32_t t2_us = hal_time_get_us();
    uint32_t real_init_time_us = t2_us - t1_us;

    (void)printf("🚀 [PERF] App Initialization Time: %lu us\n", real_init_time_us);

    // 存入黑盒子備份，確保下次當機重啟時能查閱
    g_crash_dump.boot_time_us = real_init_time_us;

    // =========================================================
    // 🌟 4. 啟動 Watchdog 防禦
    // =========================================================
    // 設定 Watchdog 倒數計時，時間到硬體將強制 Reset
    watchdog_enable(WDT_TIMEOUT_MS, 1);

    uint32_t loop_counter = 0;

    // 5. 進入無窮迴圈 (Super Loop)
    while (1)
    {
        // ⚠️ 在正式量產的程式碼中，應該在這裡呼叫 watchdog_update(); 餵狗
        // 但為了進行 Crash Dump 面試火力展示，我們今天故意不餵狗！

        app_main_task();

        loop_counter++;
        sleep_ms(MAIN_LOOP_DELAY_MS);
        (void)printf("🔄 系統正常運行中... (Loop %lu)\n", loop_counter);

        // =========================================================
        // 【面試火力展示：致命故障注入測試】
        // =========================================================
        if (loop_counter == FAULT_INJECTION_LOOP_COUNT)
        {
            (void)printf("\n😈 [FAULT INJECTION] 模擬 App 層死鎖或 HardFault...\n");

            // 寫下遺言到 .uninitialized_data 黑盒子區塊
            crash_dump_save_log("FATAL: System deadlocked! WDT will trigger soon.");

            while (1)
            {
                // 系統死鎖，卡在這裡不餵狗。
                // 等待時間達到 WDT_TIMEOUT_MS 後，RP2350 會被強制 Reset！
            }
        }
    }

    return 0;
}