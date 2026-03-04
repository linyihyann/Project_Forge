# Project Forge: Automotive-Grade RTOS-less Framework on RP2350

## 🌟 Key Features (English)

### 1. Test-Driven Development (TDD) & 100% Code Coverage
* **Host-based Testing:** Unit tests run on the host machine (macOS/Linux) using **Unity** and **CMock** via the **Ceedling** build system, executing in `< 0.5s` for instant feedback.
* **Strict Hardware Mocking:** CMock is configured with `:enforce_strict_ordering: TRUE` to mathematically guarantee the exact sequence of hardware API calls.
* **Zero-Overhead Test Hooks:** Eradicated state leakage and unsigned integer underflow in unit tests using C preprocessor `#ifdef TEST` hooks, ensuring pristine testing environments without polluting production ROM.

### 2. High-Frequency Comms & Zero-Copy DMA Ping-Pong Buffer
* **Zero-Packet-Loss Architecture:** Implemented a DMA Ping-Pong Buffer for 921600 bps UART streams. 
* **Safe Initialization Sequence:** Permanently eliminated boot-time HardFaults caused by transient noise by establishing a strict "Resource Claiming before IRQ Enable" sequence.
* **Software Idle Polling:** Bypassed the ARM PL011 hardware RX FIFO timeout limitations by introducing a 100Hz Super Loop "Idle Polling" mechanism, dropping UART CPU utilization from 60% to `< 5%`.

### 3. Lock-Free Data Structures & 10kHz Concurrency Validation (New)
* **Lock-Free Ring Buffer:** Implemented a single-producer, single-consumer ring buffer using ARM `__dmb()` (Data Memory Barrier) to prevent data corruption without blocking system Mutexes.
* **10kHz Preemption Validation:** Passed a rigorous 10,000Hz (100μs) hardware timer interrupt stress test on the RP2350 physical board, proving zero data drops, zero overwrites, and graceful backpressure handling.
* **Boot Starvation Defense:** Implemented a strict `stdio_usb_connected()` barrier to prevent TinyUSB CDC background task starvation during extreme high-frequency IRQ bursts.

---

## 🌟 核心軟體架構與亮點 (繁體中文版)

這是一個以 **Clean Architecture (乾淨架構)** 為核心設計的車規級嵌入式專案。具備 100% 跨平台移植能力、絕對可重製編譯環境 (Bit-for-bit Reproducibility) 與測試左移防禦工事 (Shift-Left Guardrails)。

### 1. 10kHz 併發壓測驗證的無鎖環形緩衝區 (Lock-Free Ring Buffer)
- **記憶體屏障同步技術**：基於單向推進指標與 ARM Cortex-M `__dmb()` 指令，實作完全不依賴 Mutex 或關中斷 (Disable IRQ) 的 Lock-free 資料結構，極大化系統即時性 (Real-time)。
- **10,000Hz 實體極限壓測**：在 RP2350 上以 100μs 週期觸發硬體中斷進行搶佔式寫入壓測。在模擬主迴圈背壓 (Backpressure) 的極限環境下，達成零斷號、零資料覆蓋、零死機的完美傳輸。

### 2. 高頻通訊與零拷貝 DMA 雙緩衝 
- **Ping-Pong Buffer 架構**：針對 921600 bps 極限傳輸，實作 DMA 雙緩衝機制解決中斷風暴。
- **軟體輪詢 (Idle Polling) 演算法**：完美迴避 ARM PL011 硬體 FIFO 互斥陷阱，將通訊 CPU 佔用率壓低至 `< 5%`。
- **消除競態條件 (Race Condition)**：透過隔離法揪出 UART RX 中斷提早觸發導致的 HardFault，確立「硬體資源先行分配，最後開啟中斷」的車規級初始化防禦紀律。

### 3. TDD 測試驅動開發與開機盲區防護
- **開機握手防禦**：實作 `stdio_usb_connected()` 屏障，徹底解決 RP2350 開機過快與中斷風暴導致的 USB CDC 驅動餓死 (Starvation) 與列舉失敗盲區。
- **Docker 化極速驗證**：導入 Docker + Ceedling (Unity/CMock) 工具鏈，將靜態分析與單元測試無縫整合至 CI Pipeline。

## 📂 專案目錄結構 (Architecture Tree)
```text
├── CMakeLists.txt        # 總裝配線 (Composition Root)
├── Dockerfile            # 跨平台無塵室構建圖紙 (Multi-stage)
├── build.sh              # 自動化編譯與跨平台權限封裝腳本
├── main.c                # 乾淨的系統進入點 (包含 USB 握手防禦)
├── .clang-format         # 車規級代碼排版規範 (Allman Style)
├── tools/                # 基礎設施與防禦工具
│   ├── git-hooks/        # 統一管理的 Git Hooks
│   │    └── pre-commit 
│   ├── mem_profiler.py   # ELF 記憶體解析與攔截腳本
│   └── mem_baseline.json # 記憶體用量歷史基準點
├── test/                 # Host 端單元測試 (TDD)
│   ├── test_app_fsm.c    # 狀態機 100% 覆蓋率測試案例
│   ├── test_app_main.c   # Super Loop 與 DMA 輪詢模擬測試
│   └── test_ring_buffer.c # Ring Buffer 邏輯與邊界測試
├── src/
│   ├── app/              # 跨平台業務邏輯層 (100% 獨立)
│   │    ├── app_fsm.c    
│   │    ├── app_fsm.h   
│   │    └── app_main.c   # 系統主任務 (包含 10kHz 壓測驗證邏輯)      
│   │    ├── app_main.h    
│   │    └── CMakeLists.txt   
│   ├── utils/            # 純邏輯基礎設施 (Data Structures)
│   │    └── ring_buffer.c # Lock-free 環形緩衝區實作
│   └── hal/              # 硬體抽象層 (Hardware Abstraction Layer)
│       ├── include/      # HAL 介面定義
│       └── rp2350/       # RP2350 實體驅動實作
│            ├── hal_time_rp2350.c  # 10kHz 硬體中斷與 __dmb() 屏障
│            ├── hal_dma_rp2350.c   # PL011 UART DMA 與競態防禦
│            ├── hal_dio_rp2350.c    
│            └── CMakeLists.txt 