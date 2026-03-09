#include <stddef.h>

#include "hal_i2c.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"  // 為了使用 sleep_us

/* 依據實體接線配置 (I2C0, SDA=GP4, SCL=GP5) */
#define I2C_PORT i2c0
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5

hal_i2c_status_t hal_i2c_init(uint32_t baudrate)
{
    i2c_init(I2C_PORT, baudrate);

    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    return HAL_I2C_OK;
}

hal_i2c_status_t hal_i2c_master_tx(uint8_t dev_addr, const uint8_t* p_data, uint16_t length,
                                   uint32_t timeout_us)
{
    hal_i2c_status_t status = HAL_I2C_OK;
    int sdk_ret;

    if ((p_data == NULL) || (length == 0U))
    {
        status = HAL_I2C_ERR_PARAM;
    }
    else
    {
        /* 呼叫帶 Timeout 的 Pico SDK API，杜絕死等 */
        sdk_ret = i2c_write_timeout_us(I2C_PORT, dev_addr, p_data, length, false, timeout_us);

        if (sdk_ret == PICO_ERROR_TIMEOUT)
        {
            status = HAL_I2C_ERR_TIMEOUT;
        }
        else if (sdk_ret < 0)
        {
            status = HAL_I2C_ERR_NACK;
        }
        else if (sdk_ret != (int)length)
        {
            status = HAL_I2C_ERR_HW;
        }
        else
        {
            status = HAL_I2C_OK;
        }
    }

    return status;
}

void hal_i2c_bus_recovery(void)
{
    /* 1. 奪取控制權：將 I2C Pinmux 強制切換為普通 GPIO */
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_SIO);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_SIO);

    gpio_set_dir(I2C_SDA_PIN, GPIO_IN);   // SDA 設為輸入，用來觀察從機是否放開
    gpio_set_dir(I2C_SCL_PIN, GPIO_OUT);  // SCL 設為輸出，準備打 Clock

    /* 2. 核心救援：打出最多 9 個 Clock */
    for (uint8_t i = 0U; i < 9U; i++)
    {
        gpio_put(I2C_SCL_PIN, 1);
        sleep_us(5);  // 模擬 ~100kHz 的半週期
        gpio_put(I2C_SCL_PIN, 0);
        sleep_us(5);

        /* 如果發現 SDA 已經被從機釋放 (變成 High)，就可以提早結束救援 */
        if (gpio_get(I2C_SDA_PIN) == 1)
        {
            break;
        }
    }

    /* 3. 善後處理：強制打出一個 STOP Condition (SCL High 的情況下，SDA 由 Low 變 High) */
    gpio_set_dir(I2C_SDA_PIN, GPIO_OUT);
    gpio_put(I2C_SCL_PIN, 0);
    sleep_us(5);
    gpio_put(I2C_SDA_PIN, 0);
    sleep_us(5);
    gpio_put(I2C_SCL_PIN, 1);
    sleep_us(5);
    gpio_put(I2C_SDA_PIN, 1);
    sleep_us(5);

    /* 4. 完璧歸趙：將腳位控制權還給 I2C 硬體控制器，並重新初始化 */
    (void)hal_i2c_init(400000U);
}