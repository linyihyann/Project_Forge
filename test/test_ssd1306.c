/* test/test_app_ssd1306.c */
#include "app_ssd1306.h"
#include "unity.h"

/* 🌟 引入 CMock 自動生成的假硬體介面 */
#include "mock_hal_i2c.h"

void setUp(void)
{
    /* 每次測試前的初始化 (此處留空即可) */
}

void tearDown(void)
{
    /* 每次測試後的清理 (此處留空即可) */
}

/* =====================================================================
 * 🧪 測試案例 1：Happy Path - 完美啟動
 * ===================================================================== */
void test_app_ssd1306_init_should_ReturnTrue_When_I2cIsOk(void)
{
    /* [Arrange] 假裝底層 I2C 永遠回傳成功 */
    hal_i2c_master_tx_IgnoreAndReturn(HAL_I2C_OK);

    /* [Act] */
    bool result = app_ssd1306_init();

    /* [Assert] */
    TEST_ASSERT_TRUE_MESSAGE(result, "Init should succeed when I2C is OK.");
}

/* =====================================================================
 * 🧪 測試案例 2：Fail-fast - 硬體卡死防禦
 * ===================================================================== */
void test_app_ssd1306_init_should_ReturnFalse_And_FailFast_When_I2cTimeout(void)
{
    /* [Arrange] 預期 App 只會呼叫 1 次 I2C (因為第一次就會拿到 TIMEOUT 並 break) */
    hal_i2c_master_tx_ExpectAnyArgsAndReturn(HAL_I2C_ERR_TIMEOUT);

    /* [Act] */
    bool result = app_ssd1306_init();

    /* [Assert] 必須攔截到錯誤並回傳 false */
    TEST_ASSERT_FALSE_MESSAGE(result,
                              "Init should return false and abort immediately on I2C Timeout.");
}

/* =====================================================================
 * 🧪 測試案例 3：Mid-flight Failure - 繪圖中途被拔線
 * ===================================================================== */
void test_app_ssd1306_fill_should_ReturnFalse_When_MidwayNack(void)
{
    /* [Arrange] 模擬：寫入第 1 個 Page 成功，寫入第 2 個 Page 時線路鬆脫 (NACK) */
    hal_i2c_master_tx_ExpectAnyArgsAndReturn(HAL_I2C_OK);       /* 第 1 次呼叫：成功 */
    hal_i2c_master_tx_ExpectAnyArgsAndReturn(HAL_I2C_ERR_NACK); /* 第 2 次呼叫：失敗 */

    /* [Act] */
    bool result = app_ssd1306_fill(0xFFU);

    /* [Assert] 必須在第 2 個 Page 發生錯誤時立刻回傳 false */
    TEST_ASSERT_FALSE_MESSAGE(result,
                              "Fill should abort and return false if a mid-flight NACK occurs.");
}