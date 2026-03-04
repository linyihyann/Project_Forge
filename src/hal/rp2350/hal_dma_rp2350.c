#include "hal_dma.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/sync.h"
#include "hardware/uart.h"

// 依據你的硬體設計配置
#define UART_ID uart0
#define UART_IRQ UART0_IRQ
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define RX_BUFFER_SIZE 2048U  // 🌟 面試亮點：足夠大以減少 Queue Full 的機率，但又不會浪費太多 RAM

// ==========================================
// 🛡️ Ping-Pong Buffer 實體與狀態變數
// ==========================================
static uint8_t buffer_A[RX_BUFFER_SIZE];
static uint8_t buffer_B[RX_BUFFER_SIZE];

static uint8_t* current_dma_buf = buffer_A;
static uint8_t* ready_app_buf = NULL;         // 🌟 補上：指向準備好給 App 的 Buffer
static volatile uint16_t ready_data_len = 0;  // 🌟 補上：該包資料的真實長度

static volatile bool is_packet_ready = false;
static hal_uart_packet_t ready_packet = {0};
static int dma_rx_chan = -1;

static void uart_rx_isr(void)
{
    // 🌟 獲取底層暫存器指標 (Pico SDK inline 優化，無效能負擔)
    uart_hw_t* hw = uart_get_hw(UART_ID);
    uint32_t mis = hw->mis;  // 讀取 Masked Interrupt Status

    bool is_timeout = (mis & UART_UARTMIS_RTMIS_BITS) != 0;
    bool is_error = (mis & (UART_UARTMIS_OEMIS_BITS | UART_UARTMIS_BEMIS_BITS |
                            UART_UARTMIS_PEMIS_BITS | UART_UARTMIS_FEMIS_BITS)) != 0;

    // ---------------------------------------------------------
    // 🛡️ 失效模式防禦：硬體錯誤 (Framing / Overrun) 處理
    // ---------------------------------------------------------
    if (is_error)
    {
        // 1. 立刻中止失控的 DMA 搬運
        dma_channel_abort(dma_rx_chan);

        // 2. Flush UART RX FIFO (將殘留的垃圾資料讀空，直到 RXFE 旗標為 1)
        while ((hw->fr & UART_UARTFR_RXFE_BITS) == 0)
        {
            (void)hw->dr;  // 讀取並丟棄
        }

        // 3. 重置 DMA 狀態，讓它重新從當前 Buffer 的開頭開始接資料
        // 注意：這裡不切換 Ping-Pong，直接把污染的 Buffer 覆蓋掉
        dma_channel_set_write_addr(dma_rx_chan, current_dma_buf, false);
        dma_channel_set_trans_count(dma_rx_chan, RX_BUFFER_SIZE, true);  // true 表示重新觸發

        // 4. 清除所有的 Error 中斷旗標 (W1C)
        hw->icr = (UART_UARTICR_OEIC_BITS | UART_UARTICR_BEIC_BITS | UART_UARTICR_PEIC_BITS |
                   UART_UARTICR_FEIC_BITS);

        // (可選) 在這裡累加 comm_error_count，供後續診斷機制使用
    }
    // ---------------------------------------------------------
    // 🎯 核心邏輯：非定長封包結束 (Timeout) 處理
    // ---------------------------------------------------------
    else if (is_timeout)
    {
        // 1. 暫停 DMA，凍結現場
        dma_channel_abort(dma_rx_chan);

        // 2. 結算長度：總容量 - DMA 通道剩餘未搬運的 Count
        // 🌟 面試亮點：透過直接讀取 DMA 暫存器達成 O(1) 長度結算
        uint32_t remaining_count = dma_hw->ch[dma_rx_chan].transfer_count;
        uint16_t received_len = (uint16_t)(RX_BUFFER_SIZE - remaining_count);

        // 3. 確保真的有收到資料，且 App 層已經把上一個封包消化完
        if ((received_len > 0) && (!is_packet_ready))
        {
            ready_data_len = received_len;
            ready_app_buf = current_dma_buf;
            is_packet_ready = true;  // 立起 Flag 給 Super Loop 抓取

            // 切換 Ping-Pong Buffer 指標
            current_dma_buf = (current_dma_buf == buffer_A) ? buffer_B : buffer_A;

            // 🌟 Tier-1 護城河：Data Memory Barrier
            // 強制將 Store Buffer 的內容寫入 RAM，避免 CPU 讀到 Cache/Pipeline 裡的舊資料
            __dmb();
        }
        else if (is_packet_ready)
        {
            // 🚨 發生 Queue Full (App 處理太慢)
            // 這裡我們選擇 Drop (丟棄新資料)，保持 current_dma_buf 不變
            // (實務上可以增加一個 overflow_count 讓 Diagnostics 追蹤)
        }

        // 4. 重啟 DMA，指向新的 (或沿用，若 Queue Full) Buffer 準備收下一包
        dma_channel_set_write_addr(dma_rx_chan, current_dma_buf, false);
        dma_channel_set_trans_count(dma_rx_chan, RX_BUFFER_SIZE, true);  // true = start

        // 5. 清除 UART Timeout 中斷旗標
        hw->icr = UART_UARTICR_RTIC_BITS;
    }
    // ---------------------------------------------------------
    // 👻 雜訊防禦：Spurious Interrupt
    // ---------------------------------------------------------
    else
    {
        // 如果沒有匹配的預期狀態，將觸發的中斷全數清除，防止中斷卡死
        hw->icr = mis;
    }
}

// ==========================================
// 🚀 初始化 UART 與 DMA (Tier-1 防禦標準)
// ==========================================
hal_uart_status_t hal_uart_dma_init(uint32_t baudrate)
{
    // 1. 初始化 UART 基本參數
    uart_init(UART_ID, baudrate);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // 設定 8N1 格式
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    // 開啟硬體 FIFO (PL011 必須開 FIFO 才能正確觸發 Timeout 中斷)
    uart_set_fifo_enabled(UART_ID, true);

    // 2. 精準配置 UART 中斷 (這是面試必考的雷區)
    // ⚠️ 絕不使用 pico-sdk 的 uart_set_irq_enables()，因為它會無差別打開 RX 中斷
    // 我們只打開 Timeout(RTIM) 和 硬體錯誤(OEIM, BEIM, PEIM, FEIM) 中斷
    uart_hw_t* hw = uart_get_hw(UART_ID);
    hw->imsc = (UART_UARTIMSC_RTIM_BITS | UART_UARTIMSC_OEIM_BITS | UART_UARTIMSC_BEIM_BITS |
                UART_UARTIMSC_PEIM_BITS | UART_UARTIMSC_FEIM_BITS);

    // 註冊我們剛寫好的硬派 ISR 到 NVIC (巢狀向量中斷控制器)
    irq_set_exclusive_handler(UART_IRQ, uart_rx_isr);
    irq_set_enabled(UART_IRQ, true);

    // 3. 配置 DMA Channel
    dma_rx_chan = dma_claim_unused_channel(true);
    if (dma_rx_chan < 0)
    {
        return HAL_UART_ERR_INIT_FAIL;  // 防禦：拿不到 DMA 通道就報錯
    }

    dma_channel_config c = dma_channel_get_default_config(dma_rx_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);  // 每次搬運 1 Byte
    channel_config_set_read_increment(&c, false);           // 來源：永遠是 UART DR 暫存器，不遞增
    channel_config_set_write_increment(&c, true);           // 目的：寫入我們的 RAM Buffer，要遞增
    channel_config_set_dreq(&c, uart_get_dreq(UART_ID, false));  // 觸發源：UART RX

    // 綁定 DMA 到暫存器，但先不啟動 (Start)
    dma_channel_configure(dma_rx_chan, &c,
                          current_dma_buf,            // 初始寫入位址
                          &uart_get_hw(UART_ID)->dr,  // 來源位址 (UART Data Register)
                          RX_BUFFER_SIZE,             // 傳輸總量
                          false                       // false = 不立刻啟動
    );

    return HAL_UART_OK;
}

// ==========================================
// 🚀 啟動背景接收
// ==========================================
hal_uart_status_t hal_uart_dma_start_rx(void)
{
    if (dma_rx_chan < 0) return HAL_UART_ERR_INIT_FAIL;
    dma_channel_start(dma_rx_chan);
    return HAL_UART_OK;
}

// ==========================================
// 🚀 軟體 Timeout 輪詢 (解決 PL011 DMA 互斥陷阱)
// ==========================================
hal_uart_status_t hal_uart_dma_get_ready_packet(hal_uart_packet_t* out_packet)
{
    if (!out_packet) return HAL_UART_ERR_NO_DATA;

    // 🌟 讀取當前 DMA 剩餘量，計算「目前已收到的總長度」
    uint32_t remaining = dma_hw->ch[dma_rx_chan].transfer_count;
    uint16_t current_len = (uint16_t)(RX_BUFFER_SIZE - remaining);

    // 紀錄上一次 (10ms 前) 看到的長度
    static uint16_t last_len = 0;

    // 【核心邏輯】：如果有收到資料，而且長度跟 10ms 前一模一樣
    // 代表這包資料已經傳完，線路閒置了！
    if ((current_len > 0) && (current_len == last_len))
    {
        // 1. 凍結 DMA，準備結算
        dma_channel_abort(dma_rx_chan);

        // 2. 結算長度與指標
        ready_data_len = current_len;
        ready_app_buf = current_dma_buf;

        // 3. Ping-Pong 切換
        current_dma_buf = (current_dma_buf == buffer_A) ? buffer_B : buffer_A;

        // 🛡️ Data Memory Barrier (防禦 CPU 讀到舊 Cache)
        __dmb();

        // 4. 重啟 DMA 指向新的 Buffer
        dma_channel_set_write_addr(dma_rx_chan, current_dma_buf, false);
        dma_channel_set_trans_count(dma_rx_chan, RX_BUFFER_SIZE, true);

        // 5. 將結果交給 App 層，並重置 last_len 準備收下一包
        last_len = 0;
        out_packet->data_ptr = ready_app_buf;
        out_packet->length = ready_data_len;
        out_packet->has_hw_error = false;

        return HAL_UART_OK;
    }

    // 更新紀錄，供下一個 10ms 比較
    last_len = current_len;
    return HAL_UART_ERR_NO_DATA;
}

// ==========================================
// 🚀 App 層用完資料後釋放 Buffer
// ==========================================
void hal_uart_dma_release_packet(void)
{
    // 解除鎖定，讓 ISR 下次可以把資料塞進這個 Ping-Pong 空間
    is_packet_ready = false;
}

// 在 hal_uart_dma_rp2350.c 新增：
hal_uart_status_t hal_uart_send(const uint8_t* data, uint16_t length)
{
    if (data == NULL || length == 0) return HAL_UART_ERR_NO_DATA;
    // 使用 Pico SDK 的阻塞發送 (因為 baudrate 高達 921600，發送速度極快)
    uart_write_blocking(UART_ID, data, length);
    return HAL_UART_OK;
}