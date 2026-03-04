#include <stdint.h>
#include <stdio.h>

#include "app_main.h"
#include "pico/stdlib.h"  // 🌟 在 main.c 裡合法且必須！

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

    // cppcheck-suppress misra-c2012-17.7
    // cppcheck-suppress misra-c2012-21.6
    printf("\n\n====================================\n");

    // cppcheck-suppress misra-c2012-17.7
    // cppcheck-suppress misra-c2012-21.6
    printf("✅ USB Serial Connected Successfully!\n");

    // cppcheck-suppress misra-c2012-17.7
    // cppcheck-suppress misra-c2012-21.6
    printf("====================================\n\n");

    // 2. 開始系統初始化 (這裡面會啟動 10kHz 壓測)
    app_main_init();

    // 3. 進入無窮迴圈
    while (1)
    {
        app_main_task();
    }
    return 0;
}