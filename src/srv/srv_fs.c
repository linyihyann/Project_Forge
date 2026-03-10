#include "srv_fs.h" /* 🌟 引入自己的名片，解決狀態碼找不到的問題 */

#include "hal_flash.h"
#include "lfs.h"

/* ========================================================================== */
/* 🌟 宣告 LittleFS 實體變數 (解決 undeclared 的錯誤)                         */
/* ========================================================================== */
static lfs_t lfs_instance;

/* ========================================================================== */
/* 轉接層 (Adapter)：將 LittleFS 的 API 請求，轉譯給我們的 HAL 層               */
/* ========================================================================== */

static int srv_hal_read_wrapper(const struct lfs_config* c, lfs_block_t block, lfs_off_t off,
                                void* buffer, lfs_size_t size)
{
    uint32_t addr = (block * c->block_size) + off;
    return (hal_flash_read(addr, buffer, size) == HAL_FLASH_OK) ? 0 : LFS_ERR_IO;
}

static int srv_hal_prog_wrapper(const struct lfs_config* c, lfs_block_t block, lfs_off_t off,
                                const void* buffer, lfs_size_t size)
{
    uint32_t addr = (block * c->block_size) + off;
    return (hal_flash_prog(addr, buffer, size) == HAL_FLASH_OK) ? 0 : LFS_ERR_IO;
}

static int srv_hal_erase_wrapper(const struct lfs_config* c, lfs_block_t block)
{
    uint32_t addr = block * c->block_size;
    return (hal_flash_erase(addr, c->block_size) == HAL_FLASH_OK) ? 0 : LFS_ERR_IO;
}

static int srv_hal_sync_wrapper(const struct lfs_config* c)
{
    (void)c; /* 防止 unused warning */
    return (hal_flash_sync() == HAL_FLASH_OK) ? 0 : LFS_ERR_IO;
}

/* ========================================================================== */
/* 檔案系統掛載與格式化邏輯                                                     */
/* ========================================================================== */
srv_fs_status_t srv_fs_init(void)
{
    /* ========================================================================== */
    /* LittleFS 硬體設定配置 (符合 RP2040/RP2350 QSPI Flash 的物理限制)             */
    /* ========================================================================== */
    static const struct lfs_config cfg = {
        .read = srv_hal_read_wrapper,
        .prog = srv_hal_prog_wrapper,
        .erase = srv_hal_erase_wrapper,
        .sync = srv_hal_sync_wrapper,

        .read_size = 1,
        .prog_size = 256,
        .block_size = 4096,
        .block_count = 128,
        .cache_size = 256,
        .lookahead_size = 16,
        .block_cycles = 500,
    };

    /* 1. 嘗試掛載檔案系統 */
    int err = lfs_mount(&lfs_instance, &cfg);

    /* 2. 如果掛載失敗，代表是全新 Flash 或結構損毀，觸發重新格式化 */
    if (err != LFS_ERR_OK)
    {
        err = lfs_format(&lfs_instance, &cfg);
        if (err != LFS_ERR_OK)
        {
            return SRV_FS_ERR_FORMAT;
        }

        /* 格式化成功後，再次嘗試掛載 */
        err = lfs_mount(&lfs_instance, &cfg);
        if (err != LFS_ERR_OK)
        {
            return SRV_FS_ERR_MOUNT;
        }
    }

    return SRV_FS_OK;
}

/* ========================================================================== */
/* 對外開放的 Getter，讓壓測模組可以取得 LittleFS 實例進行操作                */
/* ========================================================================== */
lfs_t* srv_fs_get_lfs_instance(void)
{
    return &lfs_instance;
}