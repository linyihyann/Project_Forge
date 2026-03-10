#include "app_fs_stress.h"

#include <stdio.h>
#include <string.h>

#include "lfs.h"
#include "srv_fs.h" /* 服務層是純軟體介面，可以合法 include */

#define STRESS_FILE_NAME "boot_stress.txt"
#define LOG_BUFFER_SIZE 64U
#define TARGET_CYCLES 1000U

extern lfs_t* srv_fs_get_lfs_instance(void);

static uint32_t update_and_get_boot_count(lfs_t* lfs)
{
    /* ... (此函數內容完全不變，維持你的原樣) ... */
    uint32_t boot_count = 0U;
    lfs_file_t file;
    int err = lfs_file_open(lfs, &file, STRESS_FILE_NAME, LFS_O_RDWR | LFS_O_CREAT);
    if (err == LFS_ERR_OK)
    {
        lfs_ssize_t read_len = lfs_file_read(lfs, &file, &boot_count, sizeof(boot_count));
        if (read_len != (lfs_ssize_t)sizeof(boot_count))
        {
            boot_count = 0U;
        }
        boot_count++;
        (void)lfs_file_rewind(lfs, &file);
        (void)lfs_file_write(lfs, &file, &boot_count, sizeof(boot_count));
        (void)lfs_file_sync(lfs, &file);
        (void)lfs_file_close(lfs, &file);
    }
    return boot_count;
}

/* ========================================================================== */
/* 🌟 將底層功能透過參數注入進來                                               */
/* ========================================================================== */
void app_fs_stress_execute(app_time_cb_t get_time_cb, app_system_reset_cb_t reset_cb)
{
    /* 防呆檢查：確保上層有注入依賴 */
    if ((get_time_cb == NULL) || (reset_cb == NULL))
    {
        (void)printf("[FATAL] Dependency Injection failed!\r\n");
        return;
    }

    (void)printf("\r\n--- 🚀 Storage & Power-Loss Stress Test ---\r\n");

    srv_fs_status_t status = srv_fs_init();
    if (status != SRV_FS_OK)
    {
        (void)printf("[FATAL] 檔案系統掛載與格式化皆失敗! 狀態碼: %d\r\n", status);
        while (1)
        {
        }
    }

    lfs_t* lfs = srv_fs_get_lfs_instance();
    uint32_t current_boot = update_and_get_boot_count(lfs);
    (void)printf("[INFO] LittleFS 掛載成功! 目前重啟次數: %" PRIu32 "\r\n", current_boot);

    if (current_boot >= TARGET_CYCLES)
    {
        (void)printf("\r\n🎉 [SUCCESS] 達成 %" PRIu32 " 次無預警斷電重啟！\r\n", TARGET_CYCLES);
        return;
    }

    lfs_file_t log_file;
    int err =
        lfs_file_open(lfs, &log_file, "stress.log", LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND);

    if (err == LFS_ERR_OK)
    {
        uint32_t write_counter = 0U;
        char log_buf[LOG_BUFFER_SIZE];

        while (1)
        {
            write_counter++;
            (void)snprintf(log_buf, sizeof(log_buf), "Boot: %" PRIu32 ", Write: %" PRIu32 "\r\n",
                           current_boot, write_counter);
            (void)lfs_file_write(lfs, &log_file, log_buf, strlen(log_buf));
            (void)lfs_file_sync(lfs, &log_file);
            (void)printf("已落盤: %s", log_buf);

            for (volatile int i = 0; i < 1000000; i++)
            {
            }

            uint32_t current_us = get_time_cb();
            if ((current_us % 73U) == 0U)
            {
                (void)printf("💀 [打擊注入] 模擬電源瞬間拔除 (WDT Reset)!\r\n");

                for (volatile int i = 0; i < 1000000; i++)
                {
                }

                reset_cb();
                while (1)
                {
                } /* 等待死亡 */
            }
        }
    }
}