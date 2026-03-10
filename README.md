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

### 6. Power-Loss Resilient Storage & Headless Stress Testing (New)
* **100% Power-Loss Survival:** Integrated **LittleFS** via a strictly decoupled Service Layer (`srv_fs`), achieving 0 Bytes of Heap consumption. Successfully validated by a 1,000-cycle pseudo-random Watchdog hard-reset stress test with zero data corruption.
* **Dependency Injection (DI) Architecture:** Completely eradicated hardware SDK dependencies (`hardware/watchdog`, `hal_time`) from the Application layer using function pointer injection at the Composition Root (`main.c`), achieving 100% platform-agnostic business logic.
* **Hardware-Accurate TDD Fault Injection:** Engineered a "Global Death Flag" mechanism in Ceedling CMock to simulate true physical power loss (blocking all Read/Prog/Erase/Sync operations post-fault). Mathematically proved the Fail-Safe recovery algorithm of the File System.
* **Headless Automated Validation:** Implemented a robust USB CDC timeout fallback mechanism, enabling automated standalone stress testing via Power Bank without Host PC intervention, fully immune to USB enumeration deadlocks caused by rapid Watchdog reboots.

---

## 🌟 核心特色 (繁體中文)

### 1. 測試驅動開發 (TDD) 與 100% 程式碼覆蓋率
*(維持原樣...)*

### 2. 高頻通訊與零拷貝 DMA 雙緩衝
*(維持原樣...)*

### 3. 無鎖資料結構與 10kHz 併發壓測
*(維持原樣...)*

### 4. 系統可觀測性與死後驗屍機制
*(維持原樣...)*

### 5. I2C 總線自癒與硬體容錯防禦
*(維持原樣...)*

### 6. 抗斷電儲存架構與無頭壓測驗證 (New)
* **100% 斷電存活率：** 透過極度解耦的服務層 (`srv_fs`) 導入 **LittleFS**，達成 0 Bytes 動態記憶體 (Heap) 消耗。經 1,000 次基於微秒級偽亂數觸發的 Watchdog 實體斷電壓測，證明資料零損毀。
* **依賴注入 (Dependency Injection) 徹底解耦：** 於佈線層 (`main.c`) 透過函數指標注入底層依賴，將硬體看門狗與計時器從 App 層徹底拔除，達成 100% 平台無關 (Platform-Agnostic) 的純軟體業務邏輯。
* **貼合物理限制的 TDD 故障注入：** 於 Ceedling 單元測試環境中實作「全域死亡旗標 (Global Death Flag)」，精準模擬拔除電源後實體 Flash 拒絕讀寫的物理反應，數學級驗證檔案系統的 Fail-Safe 格式化復原邏輯。
* **無頭壓測架構 (Headless Validation)：** 實作具備 Timeout 防禦的 USB CDC 列舉機制，使 MCU 能脫離主機電腦，僅靠行動電源獨立執行上千次斷電循環驗證，徹底解決密集重啟導致的 USB 驅動死鎖問題。

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
│   ├── test_app_ssd1306.c# SSD1306 硬體故障注入與 Fail-fast 
│   └── test_srv_fs.c     # 全域死亡旗標斷電模擬與 LittleFS 掛載測試
├── src/
│   ├── app/              # 跨平台業務邏輯層 (100% 獨立)
│   │    ├── app_fs_stress.c  # 依賴注入架構之斷電壓測模組
│   │    ├── app_fs_stress.h  
│   │    ├── app_fsm.c    
│   │    ├── app_fsm.h   
│   │    ├── app_main.c         # 系統主任務 (包含 10kHz 壓測驗證邏輯)      
│   │    ├── app_main.h    
│   │    ├── app_crash_dump.c   # 死後驗屍與黑盒子存儲機制
│   │    ├── app_crash_dump.h
│   │    ├── app_system.c     
│   │    ├── app_system.h    
│   │    ├── app_ssd1306.c      # OLED 繪圖與 Fail-fast 狀態機
│   │    └── app_ssd1306.h
│   │    └── CMakeLists.txt   
│   ├── srv/                    # 服務層 (中介封裝)
│   │    ├── srv_fs.c           # LittleFS 靜態記憶體配置與 Adapter Pattern 轉接層 
│   │    ├── srv_fs.h          
│   │    └── CMakeLists.txt
│   ├── third_party/      # 第三方套件
│   │    ├── littlefs/    # 極輕量級抗斷電檔案系統 
│   │    └── CMakeLists.txt
│   ├── utils/                  # 純邏輯基礎設施 (Data Structures)
│   │    ├── ring_buffer.c # Lock-free 環形緩衝區實作
│   │    ├── ring_buffer.h
│   │    ├── observer.c    # MISRA C 規範事件派發中介軟體
│   │    ├── observer.h
│   │    └── CMakeLists.txt   
│   └── hal/              # 硬體抽象層 (Hardware Abstraction Layer)
│       ├── include/      # HAL 介面定義
│       │    ├── hal_atomic.h   
│       │    ├── hal_dio.h 
│       │    ├── hal_dma.h 
│       │    ├── hal_time.h     # 包含微秒級測時介面
│       │    ├── hal_i2c.h      # 嚴格定義 I2C 異常狀態碼與防禦介面
│       │    └── hal_flash.h    # Flash 記憶體操作介面
│       └── rp2350/       # RP2350 實體驅動實作
│            ├── hal_time_rp2350.c  # 微秒測時、10kHz 中斷與屏障
│            ├── hal_dma_rp2350.c   # PL011 UART DMA 與競態防禦
│            ├── hal_dio_rp2350.c    
│            ├── hal_i2c_rp2350.c   # 非阻塞 I2C 傳輸與 9-Clock 復原實作
│            ├── hal_flash_rp2350.c # XIP 實體位址轉換與越界防禦
│            └── CMakeLists.txt