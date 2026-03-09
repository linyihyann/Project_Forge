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

### 3. Lock-Free Data Structures & 10kHz Concurrency Validation
* **Lock-Free Ring Buffer:** Implemented a single-producer, single-consumer ring buffer using ARM `__dmb()` (Data Memory Barrier) to prevent data corruption without blocking system IRQs.
* **USB CDC Enumeration Barrier:** Implemented `stdio_usb_connected()` barrier, completely resolving USB CDC driver starvation and enumeration failure blind spots caused by RP2350 booting too fast and interrupt storms.
* **Docker-based Extreme Validation:** Integrated Docker + Ceedling (Unity/CMock) toolchain, seamlessly integrating static analysis and unit testing into the CI Pipeline.

### 4. System Observability & Post-mortem Debugging
* **Zero-Overhead Crash Dump:** Implemented a Tier-1 post-mortem debugging mechanism utilizing the `.uninitialized_data` RAM section (136 Bytes). It guarantees the survival of microsecond-level timestamps and fatal log strings across Watchdog timeouts and Warm Resets without flash wear-out.
* **Deterministic Fault Injection:** Engineered a macro-driven fault injection framework synchronized with a 2000ms Watchdog Timer to mathematically guarantee deterministic system recovery and crash evidence preservation.
* **MISRA C:2012 Compliant Observer Pattern:** Decoupled system events using a static Observer Pattern array (No `malloc`). Verified via TDD boundary tests to mathematically prove immunity to buffer overflows. Resolved Rules 7.2, 10.4, and 17.7 with proper static analysis suppression handling.

### 5. I2C Bus Recovery & Hardware Fault Tolerance
* **Microsecond Non-blocking Timeout:** Replaced legacy `while()` polling loops with DWT-based non-blocking architecture. Guarantees task exit within 5ms upon sensor disconnection, preventing Super Loop deadlock and WDT resets.
* **9-Clock Bit-banging Auto-Recovery:** Implemented an ISO-26262 inspired auto-recovery state machine. Dynamically switches I2C pinmux to SIO (GPIO) to manually generate up to 9 SCL pulses to unlock stuck Slave devices (e.g., SSD1306), achieving < 2ms seamless bus self-healing without MCU reboot.
* **Host-side Fault Injection TDD:** Configured CMock to inject `HAL_I2C_ERR_TIMEOUT` and `HAL_I2C_ERR_NACK` at the HAL boundary. Mathematically verified the App layer's "Fail-fast" logic and degraded-mode operation, achieving 100% path coverage for hardware failure scenarios.

---

## 🌟 核心特色 (繁體中文)

### 1. 測試驅動開發 (TDD) 與 100% 程式碼覆蓋率
* **Host 端測試極速回饋：** 透過 **Ceedling** 構建系統，在 macOS/Linux 上使用 **Unity** 與 **CMock** 執行單元測試，達成 `< 0.5s` 的極速重構回饋。
* **嚴格硬體解耦與時序驗證：** 配置 CMock `:enforce_strict_ordering: TRUE`，嚴格驗證底層硬體 API 的呼叫順序與時序邏輯。
* **零負擔測試掛載：** 透過 C 語言巨集 `#ifdef TEST` 隔離單元測試狀態，徹底根除測試代碼污染量產唯讀記憶體 (ROM) 的風險。

### 2. 高頻通訊與零拷貝 DMA 雙緩衝
* **零丟包架構：** 為 921600 bps UART 實作 DMA Ping-Pong Buffer (雙緩衝區)，確保高頻感測資料流無損接收。
* **車規級安全啟動時序：** 確立「先註冊資源、後開啟中斷」的嚴格時序，徹底消滅 MCU 開機瞬間因雜訊引發的 HardFault。
* **軟體空閒輪詢 (Idle Polling)：** 導入 100Hz Super Loop 輪詢機制，完美繞過 ARM PL011 硬體 RX FIFO Timeout 限制，將 UART 佔用的 CPU 負載由 60% 驟降至 `< 5%`。

### 3. 無鎖資料結構與 10kHz 併發壓測
* **無鎖環形緩衝區 (Lock-Free Ring Buffer)：** 活用 ARM `__dmb()` (資料記憶體屏障) 實作單生產者、單消費者的 Ring Buffer，在不屏蔽全域中斷的嚴苛條件下防止資料競態 (Data Race)。
* **USB 列舉防禦屏障：** 實作 `stdio_usb_connected()` 握手屏障，徹底解決 RP2350 開機過快與中斷風暴導致的 USB CDC 驅動餓死 (Starvation) 與 OS 列舉失敗盲區。
* **Docker 化無塵室驗證：** 導入 Docker 容器化技術封裝 Ceedling 工具鏈，將 MISRA C 靜態分析與單元測試無縫對接至 CI/CD 自動化流水線。

### 4. 系統可觀測性與死後驗屍機制
* **零負擔崩潰黑盒子 (Crash Dump)：** 利用 136 Bytes 的 `.uninitialized_data` RAM 區段實作死後驗屍機制。確保微秒級時間戳記與致命錯誤 Log 在 Watchdog 或 HardFault 觸發重啟後依然存活，且零 Flash 抹寫損耗。
* **確定性故障注入 (Fault Injection)：** 建構基於巨集參數化的故障注入框架，並與 2000ms 獨立看門狗同步，從數學上保證系統能確定性地從死鎖中復原並保留崩潰證據。
* **符合 MISRA C 規範之觀察者模式：** 使用靜態陣列實作事件派發中介軟體 (禁用 `malloc`)。透過 TDD 邊界測試證明系統免疫緩衝區溢位，並正確配置 cppcheck 豁免以完美解決 Rule 7.2, 10.4, 17.7 等靜態分析違規。

### 5. I2C 總線自癒與硬體容錯防禦
* **微秒級非阻塞超時防禦：** 汰除傳統死等 `while()` 迴圈，實作基於 DWT 的非阻塞架構。確保感測器斷線時在 5ms 內安全退出，徹底消滅 Super Loop 假死與非預期 WDT 重啟。
* **9-Clock 總線動態解鎖：** 實作符合 ISO 26262 精神之自動復原機制。遇總線死鎖時，動態切換 I2C Pinmux 為 GPIO，手動打出最多 9 個 Clock 強制 Slave 釋放 SDA，達成 < 2ms 的無縫自癒 (Self-healing)。
* **Host 端硬體故障注入測試：** 利用 CMock 於硬體抽象邊界精準注入 Timeout 與 NACK 錯誤。在 macOS 測試環境中嚴格驗證 App 層的「快速失敗 (Fail-fast)」與降級運轉邏輯，達成 100% 硬體失效路徑覆蓋率。
---

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
│   ├── test_ring_buffer.c# Ring Buffer 邏輯與邊界測試
│   ├── test_observer.c   # Observer Pattern 邊界與空指標防禦測試
│   └── test_app_ssd1306.c# SSD1306 硬體故障注入與 Fail-fast 邊界測試
├── src/
│   ├── app/              # 跨平台業務邏輯層 (100% 獨立)
│   │    ├── app_fsm.c    
│   │    ├── app_fsm.h   
│   │    ├── app_main.c   # 系統主任務 (包含 10kHz 壓測驗證邏輯)      
│   │    ├── app_main.h    
│   │    ├── app_crash_dump.c # 死後驗屍與黑盒子存儲機制 (New)
│   │    ├── app_crash_dump.h
│   │    ├── app_system.c     
│   │    ├── app_system.h    
│   │    ├── app_ssd1306.c    # OLED 繪圖與 Fail-fast 狀態機 (New)
│   │    └── app_ssd1306.h
│   │    └── CMakeLists.txt   
│   ├── utils/            # 純邏輯基礎設施 (Data Structures)
│   │    ├── ring_buffer.c # Lock-free 環形緩衝區實作
│   │    ├── ring_buffer.h
│   │    ├── observer.c    # MISRA C 規範事件派發中介軟體 (New)
│   │    ├── observer.h
│   │    └── CMakeLists.txt   
│   └── hal/              # 硬體抽象層 (Hardware Abstraction Layer)
│       ├── include/      # HAL 介面定義
│       │    ├── hal_atomic.h   
│       │    ├── hal_dio.h 
│       │    ├── hal_dma.h 
│       │    ├── hal_time.h     # 包含微秒級測時介面
│       │    └── hal_i2c.h      # 嚴格定義 I2C 異常狀態碼與防禦介面 (New)
│       └── rp2350/       # RP2350 實體驅動實作
│            ├── hal_time_rp2350.c  # 微秒測時、10kHz 中斷與屏障
│            ├── hal_dma_rp2350.c   # PL011 UART DMA 與競態防禦
│            ├── hal_dio_rp2350.c    
│            ├── hal_i2c_rp2350.c   # 非阻塞 I2C 傳輸與 9-Clock 復原實作 (New)
│            └── CMakeLists.txt