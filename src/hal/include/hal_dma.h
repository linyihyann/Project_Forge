#ifndef HAL_UART_DMA_H
#define HAL_UART_DMA_H

#include <stdbool.h>
#include <stdint.h>

// 狀態碼定義
typedef enum
{
    HAL_UART_OK = 0,
    HAL_UART_ERR_INIT_FAIL,
    HAL_UART_ERR_NO_DATA,
    HAL_UART_ERR_OVERRUN
} hal_uart_status_t;

// 封包資料結構 (Zero-copy 給 App 層)
typedef struct
{
    uint8_t* data_ptr;  // 指向準備好的 Buffer
    uint16_t length;    // 該包資料長度
    bool has_hw_error;  // 該包接收期間是否曾發生底層硬體錯誤 (供 App 診斷)
} hal_uart_packet_t;

/**
 * @brief 初始化 UART 與 DMA (設定 Baudrate，但不啟動接收)
 */
hal_uart_status_t hal_uart_dma_init(uint32_t baudrate);

/**
 * @brief 啟動 DMA 接收，開始聆聽背景資料
 */
hal_uart_status_t hal_uart_dma_start_rx(void);

/**
 * @brief 供 Super Loop 輪詢是否有完整封包 (Non-blocking)
 * @param out_packet 若有封包，將指標與長度寫入此結構
 * @return HAL_UART_OK 表示有新封包，HAL_UART_ERR_NO_DATA 表示無
 */
hal_uart_status_t hal_uart_dma_get_ready_packet(hal_uart_packet_t* out_packet);

/**
 * @brief 釋放當前封包，將 Buffer 交還給 DMA
 * ⚠️ App 層處理完資料後「必須」呼叫此函數，否則 Ping-Pong 會卡死
 */
void hal_uart_dma_release_packet(void);

hal_uart_status_t hal_uart_send(const uint8_t* data, uint16_t length);

#endif  // HAL_UART_DMA_H