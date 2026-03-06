#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdbool.h>
#include <stdint.h>

// 強制為 2 的次方，以利 Bitwise AND 遮罩運算，避免 Modulo (%) 帶來的效能延遲
#define RB_SIZE 256U

// cppcheck-suppress misra-c2012-2.5
#define RB_MASK (RB_SIZE - 1U)

typedef enum
{
    RB_OK = 0,
    RB_FULL,
    RB_EMPTY,
    RB_ERROR_NULL
} rb_status_t;

typedef struct
{
    uint8_t buffer[RB_SIZE];
    volatile uint32_t head;  // Producer (ISR) 專屬寫入指標
    volatile uint32_t tail;  // Consumer (Main) 專屬讀取指標
} ring_buffer_t;

void rb_init(ring_buffer_t* rb);
rb_status_t rb_enqueue(ring_buffer_t* rb, uint8_t data);
rb_status_t rb_dequeue(ring_buffer_t* rb, uint8_t* data);

#endif  // RING_BUFFER_H