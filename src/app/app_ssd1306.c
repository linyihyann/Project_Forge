/* src/app/app_ssd1306.c */
#include "app_ssd1306.h"

#include "hal_i2c.h"

#define SSD1306_I2C_ADDR 0x3CU
#define TIMEOUT_US 5000U /* 5ms timeout */

/* 內部使用的單一命令寫入函數 */
static bool ssd1306_write_cmd(uint8_t cmd)
{
    uint8_t buffer[2];
    buffer[0] = 0x00U; /* Control byte: Command */
    buffer[1] = cmd;

    return (hal_i2c_master_tx(SSD1306_I2C_ADDR, buffer, 2U, TIMEOUT_US) == HAL_I2C_OK);
}

bool app_ssd1306_init(void)
{
    bool success = true;

    /* 經典 SSD1306 初始化序列 (128x64) */
    const uint8_t init_cmds[] = {0xAEU, 0x20U, 0x00U, 0x21U, 0x00U, 0x7FU, 0x22U, 0x00U,
                                 0x07U, 0x81U, 0xCFU, 0xA1U, 0xA6U, 0xA8U, 0x3FU, 0xC8U,
                                 0xD3U, 0x00U, 0xD5U, 0x80U, 0xD9U, 0xF1U, 0xDAU, 0x12U,
                                 0xDBU, 0x40U, 0x8DU, 0x14U, 0xAFU};

    for (uint16_t i = 0U; i < sizeof(init_cmds); i++)
    {
        if (ssd1306_write_cmd(init_cmds[i]) == false)
        {
            /* 🌟 [防禦核心] Fail-fast：一遇到 I2C 錯誤就立刻終止初始化 */
            success = false;
            break;
        }
    }

    return success;
}

bool app_ssd1306_fill(uint8_t pattern)
{
    /* 為了效能，一次送出一個 Page (128 bytes) + 1 byte Control */
    uint8_t buffer[129];
    buffer[0] = 0x40U; /* Control byte: Data */

    for (uint16_t i = 1U; i <= 128U; i++)
    {
        buffer[i] = pattern;
    }

    /* 總共 8 個 Page (128x64 pixels) */
    for (uint8_t page = 0U; page < 8U; page++)
    {
        if (hal_i2c_master_tx(SSD1306_I2C_ADDR, buffer, 129U, TIMEOUT_US) != HAL_I2C_OK)
        {
            /* 🌟 [防禦核心] Mid-flight Fail：繪圖到一半被拔線，立刻放棄剩餘繪圖 */
            return false;
        }
    }
    return true;
}