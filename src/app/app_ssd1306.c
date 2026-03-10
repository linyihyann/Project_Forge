#include "app_ssd1306.h"

#include <stddef.h>

#define SSD1306_I2C_ADDR 0x3CU

static const app_ssd1306_cfg_t* s_ssd1306_cfg = NULL;

/* 內部使用的單一命令寫入函數 */
static bool ssd1306_write_cmd(uint8_t cmd)
{
    uint8_t buffer[2];
    buffer[0] = 0x00U; /* Control byte: Command */
    buffer[1] = cmd;

    return s_ssd1306_cfg->i2c_tx(SSD1306_I2C_ADDR, buffer, 2U);
}

bool app_ssd1306_init(const app_ssd1306_cfg_t* cfg)
{
    if ((cfg == NULL) || (cfg->i2c_tx == NULL))
    {
        return false;
    }
    s_ssd1306_cfg = cfg;

    const uint8_t init_cmds[] = {0xAEU, 0x20U, 0x00U, 0x21U, 0x00U, 0x7FU, 0x22U, 0x00U,
                                 0x07U, 0x81U, 0xCFU, 0xA1U, 0xA6U, 0xA8U, 0x3FU, 0xC8U,
                                 0xD3U, 0x00U, 0xD5U, 0x80U, 0xD9U, 0xF1U, 0xDAU, 0x12U,
                                 0xDBU, 0x40U, 0x8DU, 0x14U, 0xAFU};

    for (uint16_t i = 0U; i < sizeof(init_cmds); i++)
    {
        if (ssd1306_write_cmd(init_cmds[i]) == false)
        {
            return false;
        }
    }
    return true;
}

bool app_ssd1306_fill(uint8_t pattern)
{
    if (s_ssd1306_cfg == NULL)
    {
        return false;
    }

    uint8_t buffer[129];
    buffer[0] = 0x40U;
    for (uint16_t i = 1U; i <= 128U; i++)
    {
        buffer[i] = pattern;
    }

    for (uint8_t page = 0U; page < 8U; page++)
    {
        if (s_ssd1306_cfg->i2c_tx(SSD1306_I2C_ADDR, buffer, 129U) == false)
        {
            return false;
        }
    }
    return true;
}