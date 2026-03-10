#ifndef SRV_FS_H
#define SRV_FS_H

#include <stdbool.h>
#include <stdint.h>

/* *
 * [MISRA C Dir 4.6] 明確型別
 * 服務層的錯誤碼，對應用層隱藏硬體與 LittleFS 專屬的錯誤碼
 */
typedef enum
{
    SRV_FS_OK = 0,
    SRV_FS_ERR_MOUNT = -1,  /* 掛載失敗 */
    SRV_FS_ERR_FORMAT = -2, /* 格式化失敗 */
    SRV_FS_ERR_HW = -3      /* 底層硬體或 IO 錯誤 */
} srv_fs_status_t;

/* *
 * @brief 初始化並掛載檔案系統 (若無檔案系統則自動格式化)
 * @return 執行結果狀態碼
 */
srv_fs_status_t srv_fs_init(void);

/* TODO: 後續可以再加入 srv_fs_log_write 等針對 App 邏輯封裝的 API */

/* 記得在 srv_fs.h 最上方 #include "lfs.h" */
struct lfs_config; /* 前置宣告 */

/* 🌟 [修正] 使用標準的 Opaque Struct 宣告，避開 typedef 名稱衝突 */
struct lfs;

/* 🌟 [修正] 回傳標準的 struct lfs 指標 */
struct lfs* srv_fs_get_lfs_instance(void);

#endif /* SRV_FS_H */