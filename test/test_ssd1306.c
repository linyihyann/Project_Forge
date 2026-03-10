/* test/test_app_ssd1306.c */
#include "app_ssd1306.h"
#include "unity.h"

// =====================================================================
// 🌟 測試用假硬體 (Test Double)
// =====================================================================
static bool g_fake_tx_should_succeed = true;
static uint32_t g_fake_tx_call_count = 0U;
static uint32_t g_fake_tx_fail_on_call = 0U;

static bool fake_i2c_tx(uint8_t dev_addr, const uint8_t* p_data, uint16_t length)
{
    (void)dev_addr;
    (void)p_data;
    (void)length;

    g_fake_tx_call_count++;

    if ((g_fake_tx_fail_on_call != 0U) && (g_fake_tx_call_count >= g_fake_tx_fail_on_call))
    {
        return false;
    }
    return g_fake_tx_should_succeed;
}

static const app_ssd1306_cfg_t g_test_cfg = {.i2c_tx = fake_i2c_tx};

// =====================================================================
void setUp(void)
{
    g_fake_tx_should_succeed = true;
    g_fake_tx_call_count = 0U;
    g_fake_tx_fail_on_call = 0U;
}

void tearDown(void) {}

// =====================================================================
// 🧪 TC-01：Happy Path
// =====================================================================
void test_app_ssd1306_init_should_ReturnTrue_When_I2cIsOk(void)
{
    bool result = app_ssd1306_init(&g_test_cfg);
    TEST_ASSERT_TRUE_MESSAGE(result, "Init should succeed when I2C is OK.");
}

// =====================================================================
// 🧪 TC-02：Fail-fast
// =====================================================================
void test_app_ssd1306_init_should_ReturnFalse_And_FailFast_When_I2cTimeout(void)
{
    g_fake_tx_fail_on_call = 1U;

    bool result = app_ssd1306_init(&g_test_cfg);

    TEST_ASSERT_FALSE_MESSAGE(result,
                              "Init should return false and abort immediately on I2C error.");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, g_fake_tx_call_count,
                                     "Should stop immediately after first I2C failure.");
}

// =====================================================================
// 🧪 TC-03：Mid-flight Failure
// =====================================================================
void test_app_ssd1306_fill_should_ReturnFalse_When_MidwayNack(void)
{
    (void)app_ssd1306_init(&g_test_cfg);

    g_fake_tx_call_count = 0U;
    g_fake_tx_fail_on_call = 2U;

    bool result = app_ssd1306_fill(0xFFU);

    TEST_ASSERT_FALSE_MESSAGE(result,
                              "Fill should abort and return false if a mid-flight error occurs.");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2U, g_fake_tx_call_count,
                                     "Should stop at the second page on NACK.");
}