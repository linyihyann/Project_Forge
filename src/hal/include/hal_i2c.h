#ifndef HAL_I2C_H
#define HAL_I2C_H

#include <stdint.h>

/* 定義嚴格的 I2C 狀態碼 */
typedef enum
{
    HAL_I2C_OK = 0U,
    HAL_I2C_ERR_TIMEOUT,
    HAL_I2C_ERR_NACK,
    HAL_I2C_ERR_PARAM,
    HAL_I2C_ERR_HW
} hal_i2c_status_t;

/* 初始化 I2C (綁定 SDA/SCL Pinmux) */
hal_i2c_status_t hal_i2c_init(uint32_t baudrate);

/* MISRA C: 強制檢查回傳值，保護唯讀指標 */
__attribute__((warn_unused_result)) hal_i2c_status_t hal_i2c_master_tx(uint8_t dev_addr,
                                                                       const uint8_t* p_data,
                                                                       uint16_t length,
                                                                       uint32_t timeout_us);

/* 執行 9-clock I2C 總線復原機制 */
void hal_i2c_bus_recovery(void);

#endif /* HAL_I2C_H */