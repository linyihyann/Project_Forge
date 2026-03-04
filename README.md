## 🌟 Key Features (English)

### 1. Test-Driven Development (TDD) & 100% Code Coverage
* **Host-based Testing:** Unit tests run on the host machine (macOS/Linux) using **Unity** and **CMock** via the **Ceedling** build system, executing in `< 0.5s` for instant feedback.
* **Strict Hardware Mocking:** CMock is configured with `:enforce_strict_ordering: TRUE` to mathematically guarantee the exact sequence of hardware API calls.
* **Zero-Overhead Test Hooks:** Eradicated state leakage and unsigned integer underflow in unit tests using C preprocessor `#ifdef TEST` hooks, ensuring pristine testing environments without polluting production ROM.

### 2. Deterministic State Machine (FSM) & Fail-safe Design
* **Non-blocking Super Loop:** Eradicated all `delay()` functions. Driven by a 100Hz periodic tick, liberating CPU cycles and preventing starvation of the wireless coprocessor.
* **Fail-safe Mechanisms:** The FSM includes a strict 500ms timeout during the `SELF_TEST` state. Any hardware initialization failure or timeout immediately forces the system into a safe `FAULT` state.

### 3. High-Frequency Comms & Zero-Copy DMA Ping-Pong Buffer (New)
* **Zero-Packet-Loss Architecture:** Implemented a DMA Ping-Pong Buffer for 921600 bps UART streams. Defended against Data Races using ARM `__dmb()` (Data Memory Barrier).
* **Software Idle Polling:** Bypassed the ARM PL011 hardware RX FIFO timeout limitations by introducing a 100Hz Super Loop "Idle Polling" mechanism, dropping UART CPU utilization from 60% to `< 5%`.

---

## 🌟 核心軟體架構與亮點 (繁體中文版)

這是一個以 **Clean Architecture (乾淨架構)** 為核心設計的車規級嵌入式專案。具備 100% 跨平台移植能力、絕對可重製編譯環境 (Bit-for-bit Reproducibility) 與測試左移防禦工事 (Shift-Left Guardrails)。

### 1. 高頻通訊與零拷貝 DMA 雙緩衝 (Day 6 新增)
- **Ping-Pong Buffer 架構**：針對 921600 bps 極限傳輸，實作 DMA 雙緩衝機制解決中斷風暴，並嚴格插入 `__dmb()` 記憶體屏障防止 CPU 快取非同步。
- **軟體輪詢 (Idle Polling) 演算法**：完美迴避 ARM PL011 硬體 FIFO 互斥陷阱，將邊界偵測交由 Super Loop 處理，成功將通訊 CPU 佔用率壓低至 `< 5%`。

### 2. TDD 測試驅動開發與進階防禦 (Day 6 升級)
- **極速 Host 端驗證**：導入 `Ceedling / Unity / CMock` 工具鏈，在 macOS 上僅需 **0.5 秒** 即可完成全部極端邊界測試。
- **Test Hook 狀態隔離**：利用條件編譯 (`#ifdef TEST`) 建立測試後門，根除 C 語言 `static` 變數帶來的測試狀態污染 (State Leakage) 與無號整數溢位 (Underflow) 隱患，達成 100% 乾淨的量產二進位檔。

### 3. 車規級狀態機與 Fail-safe 安全防護
- **非阻塞超級迴圈 (Non-blocking)**：全面拔除阻塞式的 `delay()`，利用時間戳與 100Hz Tick 驅動狀態機，完美解決即時性 (Real-time) 瓶頸。
- **故障注入與安全降級**：實作 `SELF_TEST` 500ms 超時斷電防護，並具備底層硬體損壞的異常捕捉與 `FAULT` 狀態降級機制，符合 ISO 26262 精神。

## 📂 專案目錄結構 (Architecture Tree)
```text
├── CMakeLists.txt        # 總裝配線 (Composition Root)
├── Dockerfile            # 跨平台無塵室構建圖紙 (Multi-stage)
├── build.sh              # 自動化編譯與跨平台權限封裝腳本
├── main.c                # 乾淨的系統進入點
├── .clang-format         # 車規級代碼排版規範 (Allman Style)
├── tools/                # 基礎設施與防禦工具
│   ├── git-hooks/        # 統一管理的 Git Hooks
│   │    └── pre-commit 
│   ├── mem_profiler.py   # ELF 記憶體解析與攔截腳本
│   └── mem_baseline.json # 記憶體用量歷史基準點
├── test/                 # Host 端單元測試 (TDD)
│   ├── test_app_fsm.c    # 狀態機 100% 覆蓋率測試案例
│   ├── test_app_main.c   # Super Loop 與 DMA 輪詢模擬測試
│   └── support/
├── src/
│   ├── app/              # 跨平台業務邏輯層 (100% 獨立)
│   │    ├── app_fsm.c    # 核心狀態機 (Self-test, Heartbeat, Fail-safe)
│   │    └── app_main.c   # 系統主任務 (Non-blocking Super Loop)      
│   │    ├── app_main.h    
│   │    └── CMakeLists.txt   
│   └── hal/              # 硬體抽象層 (Hardware Abstraction Layer)
│       ├── include/      # HAL 介面定義 (統一回傳 Status Code)
│       │    ├── hal_dio.h      
│       │    ├── hal_time.h     
│       │    └── hal_dma.h      # DMA 通訊介面
│       └── rp2350/       # RP2350 實體驅動實作 (CYW43 LED, Timer)
│            ├── hal_dio_rp2350.c    
│            ├── hal_time_rp2350.c  
│            ├──hal_dma_rp2350.c    # PL011 UART DMA 實作與 Idle Polling
│            └── CMakeLists.txt 