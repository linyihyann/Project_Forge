#include <stdbool.h>
#include <string.h>

#include "mock_hal_flash.h"
#include "srv_fs.h"
#include "unity.h"

/* ========================================================================== */
/* 🌟 [架構師的 Ceedling 魔法]
 * 告訴 Ceedling：把真正的 LittleFS 抓進來一起編譯與連結，
 * 因為我們要測試的是真實的檔案系統演算法，而不是假冒的 Mock！
 * ========================================================================== */
#include "lfs.h"
#include "lfs_util.h"

/* ========================================================================== */
/* 測試環境設定 (RAM Disk Fake Object)                                        */
/* ========================================================================== */
#define MOCK_FLASH_SIZE 0x80000u /* 512KB */
static uint8_t mock_flash_memory[MOCK_FLASH_SIZE];

/* ⚡️ 故障注入控制器 (Fault Injection Controllers) */
static bool is_dead = false;         /* 全域死亡旗標：模擬電源被拔掉 */
static int prog_fail_countdown = -1; /* 寫入成功次數倒數器 */

/* ========================================================================== */
/* 硬體行為模擬 (Stubs - 模擬真實 SPI Flash 的物理反應)                         */
/* ========================================================================== */

static hal_flash_status_t stub_hal_flash_read(uint32_t addr, void* buffer, uint32_t size,
                                              int num_calls)
{
    (void)num_calls;
    if (is_dead) return HAL_FLASH_ERR_POWER_LOSS; /* 💀 斷電後，晶片沒有供電無法讀取 */

    if ((addr + size) > MOCK_FLASH_SIZE) return HAL_FLASH_ERR_ARG;
    memcpy(buffer, &mock_flash_memory[addr], size);
    return HAL_FLASH_OK;
}

static hal_flash_status_t stub_hal_flash_prog(uint32_t addr, const void* buffer, uint32_t size,
                                              int num_calls)
{
    (void)num_calls;
    if (is_dead) return HAL_FLASH_ERR_POWER_LOSS; /* 💀 斷電後，無法驅動寫入電壓 */

    /* ⚡️ 故障注入邏輯 */
    if (prog_fail_countdown > 0)
    {
        prog_fail_countdown--;
    }
    else if (prog_fail_countdown == 0)
    {
        is_dead = true; /* 💀 倒數歸零，瞬間觸發全域斷電死亡！ */
        return HAL_FLASH_ERR_POWER_LOSS;
    }

    if ((addr + size) > MOCK_FLASH_SIZE) return HAL_FLASH_ERR_ARG;
    memcpy(&mock_flash_memory[addr], buffer, size);
    return HAL_FLASH_OK;
}

static hal_flash_status_t stub_hal_flash_erase(uint32_t addr, uint32_t size, int num_calls)
{
    (void)num_calls;
    if (is_dead) return HAL_FLASH_ERR_POWER_LOSS; /* 💀 斷電後，無法驅動高壓擦除 */

    if ((addr + size) > MOCK_FLASH_SIZE) return HAL_FLASH_ERR_ARG;
    memset(&mock_flash_memory[addr], 0xFF, size);
    return HAL_FLASH_OK;
}

static hal_flash_status_t stub_hal_flash_sync(int num_calls)
{
    (void)num_calls;
    if (is_dead) return HAL_FLASH_ERR_POWER_LOSS; /* 💀 斷電後，SPI 總線無回應 */
    return HAL_FLASH_OK;
}

/* ========================================================================== */
/* 測試框架標準 Setup / TearDown                                              */
/* ========================================================================== */
void setUp(void)
{
    /* 每次測試前，將整塊 RAM Disk 抹除為 0xFF (模擬全新出廠的 Blank Flash) */
    memset(mock_flash_memory, 0xFF, MOCK_FLASH_SIZE);

    /* 重置故障注入狀態 */
    is_dead = false;
    prog_fail_countdown = -1;

    /* 將我們的 Stub 邏輯掛載到 Ceedling 產生的 Mock 介面上 */
    hal_flash_read_Stub(stub_hal_flash_read);
    hal_flash_prog_Stub(stub_hal_flash_prog);
    hal_flash_erase_Stub(stub_hal_flash_erase);
    hal_flash_sync_Stub(stub_hal_flash_sync);
}

void tearDown(void)
{
    /* 測試結束後的清理動作 (暫無) */
}

/* ========================================================================== */
/* 測試案例 (Test Cases)                                                      */
/* ========================================================================== */

/* 測試 1：【正常流程】全新的空白 Flash，應該要觸發 lfs_format 並成功掛載 */
void test_srv_fs_init_should_FormatAndMount_when_FlashIsBlank(void)
{
    srv_fs_status_t status = srv_fs_init();

    TEST_ASSERT_EQUAL(SRV_FS_OK, status);
}

/* 測試 2：【斷電注入測試】在格式化過程中的第 2 次寫入瞬間，觸發硬體斷電錯誤 */
void test_srv_fs_init_should_FailGracefully_when_PowerLossDuringFormat(void)
{
    /* * 💡 架構師筆記：
     * LittleFS 建立空磁碟只需要寫入 2 次 Superblock。
     * 我們將 countdown 設為 1，代表第 1 次寫入放行，第 2 次瞬間拔除電源 (引發全域死亡)
     */
    prog_fail_countdown = 1;

    srv_fs_status_t status = srv_fs_init();

    /* 驗證：因為格式化到一半系統死亡，必須安全攔截錯誤並回傳 SRV_FS_ERR_FORMAT (-2) */
    TEST_ASSERT_EQUAL(SRV_FS_ERR_FORMAT, status);
}