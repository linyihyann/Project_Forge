#ifndef HAL_FLASH_H
#define HAL_FLASH_H

#include <stdint.h>

/* * 錯誤碼定義
 * [MISRA C Dir 4.6] 使用明確基礎型別 (透過 enum 確保型別安全)
 * [面試亮點] 避免使用 Magic Numbers，並預留 POWER_LOSS 供軟體注入測試使用。
 */
typedef enum
{
    HAL_FLASH_OK = 0,
    HAL_FLASH_ERR_ARG = -1,       /* 參數錯誤 (非對齊位址或長度) */
    HAL_FLASH_ERR_HW = -2,        /* 硬體或 SPI 總線錯誤 */
    HAL_FLASH_ERR_TIMEOUT = -3,   /* 輪詢等待超時 */
    HAL_FLASH_ERR_POWER_LOSS = -4 /* 斷電或人為注入錯誤 (用於 TDD) */
} hal_flash_status_t;

/* * 介面定義：讀取 Flash
 * @param addr   實體位址 (需對齊硬體限制)
 * @param buffer 接收緩衝區
 * @param size   讀取大小
 */
hal_flash_status_t hal_flash_read(uint32_t addr, void* buffer, uint32_t size);

/* * 介面定義：寫入 Flash (Program)
 * [MISRA C Rule 8.13] 不會被修改的指標參數必須加上 const 宣告。
 * @param addr   實體位址 (必須為 256 Bytes 對齊)
 * @param buffer 欲寫入的資料
 * @param size   寫入大小
 */
hal_flash_status_t hal_flash_prog(uint32_t addr, const void* buffer, uint32_t size);

/* * 介面定義：擦除 Flash (Erase)
 * @param addr   實體位址 (必須為 4096 Bytes 對齊)
 * @param size   擦除大小 (必須為 4096 Bytes 的倍數)
 */
hal_flash_status_t hal_flash_erase(uint32_t addr, uint32_t size);

/* * 介面定義：同步 Flash (Sync)
 * 確保資料已確實寫入實體 Flash (等待 WIP - Write In Progress bit 清除)
 */
hal_flash_status_t hal_flash_sync(void);

#endif /* HAL_FLASH_H */