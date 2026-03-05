#include "mock_hal_atomic.h"  // 由 Ceedling 根據 hal_atomic.h 自動生成
#include "ring_buffer.h"
#include "unity.h"

static ring_buffer_t rb;

void setUp(void)
{
    // 🌟 這裡是測試 Ring Buffer 本身，所以要真的初始化！
    rb_init(&rb);
}

void tearDown(void) {}

void test_rb_enqueue_dequeue_basic(void)
{
    uint8_t read_data = 0;

    // 預期：Enqueue 和 Dequeue 都會觸發一次 DMB
    hal_atomic_dmb_Expect();
    TEST_ASSERT_EQUAL(RB_OK, rb_enqueue(&rb, 0xAA));

    hal_atomic_dmb_Expect();
    TEST_ASSERT_EQUAL(RB_OK, rb_dequeue(&rb, &read_data));

    TEST_ASSERT_EQUAL(0xAA, read_data);
}

void test_rb_full_drop_newest(void)
{
    // 寫入 255 筆資料 (RB_SIZE - 1)
    for (int i = 0; i < RB_SIZE - 1; i++)
    {
        hal_atomic_dmb_Expect();
        TEST_ASSERT_EQUAL(RB_OK, rb_enqueue(&rb, i));
    }

    // 第 256 筆應該回傳 FULL，且不會觸發 DMB
    TEST_ASSERT_EQUAL(RB_FULL, rb_enqueue(&rb, 0xFF));
}

void test_rb_empty(void)
{
    uint8_t read_data = 0;
    // 初始狀態讀取應該回傳 EMPTY
    TEST_ASSERT_EQUAL(RB_EMPTY, rb_dequeue(&rb, &read_data));
}

void test_rb_wrap_around(void)
{
    // 刻意製造指標繞回 (Wrap-around) 的情境
    // 先寫入 200 筆，再讀出 200 筆 (指標推到 200)
    uint8_t dummy;
    for (int i = 0; i < 200; i++)
    {
        hal_atomic_dmb_Expect();
        rb_enqueue(&rb, i);
    }
    for (int i = 0; i < 200; i++)
    {
        hal_atomic_dmb_Expect();
        rb_dequeue(&rb, &dummy);
    }

    // 再寫 100 筆，此時指標一定會跨越 256 邊界發生繞回
    for (int i = 0; i < 100; i++)
    {
        hal_atomic_dmb_Expect();
        TEST_ASSERT_EQUAL(RB_OK, rb_enqueue(&rb, i));
    }

    // 驗證繞回後資料正確性
    for (int i = 0; i < 100; i++)
    {
        hal_atomic_dmb_Expect();
        TEST_ASSERT_EQUAL(RB_OK, rb_dequeue(&rb, &dummy));
        TEST_ASSERT_EQUAL(i, dummy);
    }
}