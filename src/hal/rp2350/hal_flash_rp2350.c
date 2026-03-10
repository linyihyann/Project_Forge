#include <string.h>

#include "hal_flash.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

/* 實體 Flash 分配配置 */
#define XIP_BASE_ADDR 0x10000000u
#define FLASH_TOTAL_SIZE 0x400000u
#define LFS_PARTITION_SIZE 0x80000u
#define LFS_START_OFFSET (FLASH_TOTAL_SIZE - LFS_PARTITION_SIZE)

/* ========================================================================== */
/* 🛡️ [修正] 邊界防禦：這裡的 offset 應該是「相對於分割區起點的位址 (0 ~ 512KB)」 */
/* ========================================================================== */
static bool is_valid_flash_range(uint32_t offset, uint32_t size)
{
    /* 確保 LittleFS 傳來的相對位址不會超出 512KB 的配額 */
    return ((offset + size) <= LFS_PARTITION_SIZE);
}

/* -------------------------------------------------------------------------- */
/* HAL 介面實作 (將相對位址轉換為絕對物理位址)                                  */
/* -------------------------------------------------------------------------- */

hal_flash_status_t hal_flash_read(uint32_t addr, void* buffer, uint32_t size)
{
    if ((buffer == NULL) || (size == 0u) || !is_valid_flash_range(addr, size))
    {
        return HAL_FLASH_ERR_ARG;
    }

    /* 🌟 將 LittleFS 的相對位址 (addr) 加上分割區起點，轉成物理位址 */
    uint32_t physical_addr = LFS_START_OFFSET + addr;

    const uint8_t* flash_ptr = (const uint8_t*)(XIP_BASE_ADDR + physical_addr);
    (void)memcpy(buffer, flash_ptr, size);

    return HAL_FLASH_OK;
}

hal_flash_status_t hal_flash_prog(uint32_t addr, const void* buffer, uint32_t size)
{
    if ((buffer == NULL) || (size == 0u) || !is_valid_flash_range(addr, size))
    {
        return HAL_FLASH_ERR_ARG;
    }

    if (((addr % FLASH_PAGE_SIZE) != 0u) || ((size % FLASH_PAGE_SIZE) != 0u))
    {
        return HAL_FLASH_ERR_ARG;
    }

    /* 🌟 轉換物理位址 */
    uint32_t physical_addr = LFS_START_OFFSET + addr;

    uint32_t saved_interrupts = save_and_disable_interrupts();

    /* 傳給 Pico SDK 的是實際的物理位址 */
    flash_range_program(physical_addr, (const uint8_t*)buffer, size);

    restore_interrupts(saved_interrupts);

    return HAL_FLASH_OK;
}

hal_flash_status_t hal_flash_erase(uint32_t addr, uint32_t size)
{
    if ((size == 0u) || !is_valid_flash_range(addr, size))
    {
        return HAL_FLASH_ERR_ARG;
    }

    if (((addr % FLASH_SECTOR_SIZE) != 0u) || ((size % FLASH_SECTOR_SIZE) != 0u))
    {
        return HAL_FLASH_ERR_ARG;
    }

    /* 🌟 轉換物理位址 */
    uint32_t physical_addr = LFS_START_OFFSET + addr;

    uint32_t saved_interrupts = save_and_disable_interrupts();
    flash_range_erase(physical_addr, size);
    restore_interrupts(saved_interrupts);

    return HAL_FLASH_OK;
}

hal_flash_status_t hal_flash_sync(void)
{
    return HAL_FLASH_OK;
}