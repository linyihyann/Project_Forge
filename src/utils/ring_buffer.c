#include <stddef.h>

#include "hal_atomic.h"
#include "ring_buffer.h"

void rb_init(ring_buffer_t* rb)
{
    if (rb != NULL)
    {
        rb->head = 0;
        rb->tail = 0;
    }
}

rb_status_t rb_enqueue(ring_buffer_t* rb, uint8_t data)
{
    if (rb == NULL)
    {
        return RB_ERROR_NULL;
    }

    // 計算下一個 Head 的位置
    uint32_t next_head = (rb->head + 1) & RB_MASK;

    // Drop Newest 滿載策略：若下一個 Head 撞到 Tail，代表已滿
    if (next_head == rb->tail)
    {
        return RB_FULL;
    }

    rb->buffer[rb->head] = data;

    // 🔥 車規防禦點：Memory Barrier
    // 確保「資料寫入 Buffer」的動作，絕對發生在「更新 Head 指標」之前
    // 防止編譯器 -O3 優化或 CPU Pipeline 亂序執行導致的 Data Race
    hal_atomic_dmb();

    rb->head = next_head;
    return RB_OK;
}

rb_status_t rb_dequeue(ring_buffer_t* rb, uint8_t* data)
{
    if (rb == NULL || data == NULL)
    {
        return RB_ERROR_NULL;
    }

    // 若 Head 等於 Tail，代表空載
    if (rb->head == rb->tail)
    {
        return RB_EMPTY;
    }

    *data = rb->buffer[rb->tail];

    // 🔥 車規防禦點：Memory Barrier
    // 確保「讀取資料」的動作完成後，才釋放該空間 (推進 Tail 指標)
    hal_atomic_dmb();

    rb->tail = (rb->tail + 1) & RB_MASK;
    return RB_OK;
}