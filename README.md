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

### 7. Asymmetric Multi-Processing (AMP) RTOS Architecture & 10kHz IPC
* **Physical Task Isolation:** Engineered a deterministic AMP architecture on the Cortex-M33 (RP2350). Core 0 executes FreeRTOS for soft-real-time business logic (I2C, USB), while Core 1 operates as a 100% bare-metal Hard Real-Time sensing loop, mathematically eliminating OS context-switch jitter (< 10 ns latency).
* **Zero-Drop 10kHz Lock-Free IPC:** Achieved a flawless 10,000 Hz cross-core communication pipeline using a Single-Producer-Single-Consumer (SPSC) Lock-Free Ring Buffer, completely preventing Priority Inversion caused by traditional RTOS Mutexes.
* **Deferred Initialization Pattern:** Eradicated boot-time `HardFault` race conditions between Pico SDK hardware interrupts and FreeRTOS vector tables (VTOR) by delegating all hardware and USB CDC enumerations to a self-destructing `init_task`.

### 8. Microsecond Observability & Hard Real-Time Scheduling (New)
* **Microsecond Profiling & Memory Optimization:** Integrated Cortex-M33 hardware timer (`time_us_32`) with FreeRTOS Run-Time Stats. Implemented Stack High Water Mark telemetry, mathematically identifying and eliminating 90% of preemptive stack waste, strictly bounding task margins to ~230 Words.
* **Deterministic Hard Real-Time Execution:** Identified a 4% cycle-drift anomaly caused by priority interference in relative `vTaskDelay`. Enforced absolute periodicity using `srv_os_delay_until` and pure CPU dummy workloads, perfectly restoring the sensor task's CPU utilization to a deterministic 21.1%.
* **Strict OS Decoupling & Host-side Injection:** Eradicated `#include "FreeRTOS.h"` from the application layer. Engineered the `:return_thru_ptr` CMock plugin in `project.yml` to inject mock memory metrics via pointers, achieving 100% branch coverage for Limp-Home mode triggers on the Host PC.
* **MISRA C:2012 Compliance Validation:** Resolved critical static analysis violations (Rules 9.3, 15.6, 20.9) by eliminating implicit array initialization and enforcing explicit bounds checking, achieving a 100% clean CI pipeline pass.

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

### 6. 抗斷電儲存架構與無頭壓測驗證 (New)
* **100% 斷電存活率：** 透過極度解耦的服務層 (`srv_fs`) 導入 **LittleFS**，達成 0 Bytes 動態記憶體 (Heap) 消耗。經 1,000 次基於微秒級偽亂數觸發的 Watchdog 實體斷電壓測，證明資料零損毀。
* **依賴注入 (Dependency Injection) 徹底解耦：** 於佈線層 (`main.c`) 透過函數指標注入底層依賴，將硬體看門狗與計時器從 App 層徹底拔除，達成 100% 平台無關 (Platform-Agnostic) 的純軟體業務邏輯。
* **貼合物理限制的 TDD 故障注入：** 於 Ceedling 單元測試環境中實作「全域死亡旗標 (Global Death Flag)」，精準模擬拔除電源後實體 Flash 拒絕讀寫的物理反應，數學級驗證檔案系統的 Fail-Safe 格式化復原邏輯。
* **無頭壓測架構 (Headless Validation)：** 實作具備 Timeout 防禦的 USB CDC 列舉機制，使 MCU 能脫離主機電腦，僅靠行動電源獨立執行上千次斷電循環驗證，徹底解決密集重啟導致的 USB 驅動死鎖問題。

### 7. 非對稱多處理 (AMP) 雙核架構與 10kHz 極限跨核通訊 (New)
* **物理級任務隔離：** 於 Cortex-M33 (RP2350) 實作確定性 AMP 架構。Core 0 專職運行 FreeRTOS 處理業務邏輯，Core 1 降級為 100% 裸機 (Bare-metal) 執行硬即時任務，從物理層面消滅 OS 排程抖動，達成 < 10 ns 的極限觸發延遲。
* **零丟包 10kHz 無鎖通訊 (Lock-Free IPC)：** 採用單一生產/消費者 (SPSC) 無鎖環形佇列進行跨核通訊。在每秒一萬次的極端壓測下，透過內部校驗演算法證實 0% 丟包率與資料損毀，徹底根絕傳統 RTOS 互斥鎖引發的優先權反轉 (Priority Inversion)。
* **延遲初始化安全點火 (Deferred Init)：** 將底層硬體與 USB 列舉全數封裝於最高優先級的 `init_task` 中，待 OS 中斷向量表穩固後再喚醒周邊，最後自我銷毀釋放記憶體。完美解決 Pico SDK 與 FreeRTOS 在開機瞬間搶奪硬體資源導致的 HardFault 死機陷阱。

### 8. 微秒級系統可觀測性與硬即時排程 (New)
* **微秒級效能剖析與堆疊最佳化：** 將 Cortex-M33 硬體計時器 (`time_us_32`) 綁定至 FreeRTOS 核心統計，提供 1µs 精度的 CPU 負載監測。透過 High Water Mark 遙測，精準抓出並消除高達 90% 的 Stack 盲目配置浪費，將安全裕度收斂至約 230 Words。
* **硬即時 (Hard Real-Time) 週期強制執行：** 透過精準測時，抓出相對延遲 (`vTaskDelay`) 因高優先級搶佔所引發的 4% 週期飄移與 Deadline Miss。全面導入絕對延遲 (`srv_os_delay_until`) 與純 CPU 運算負載模擬，成功將感測器任務負載率精準校正回確定性的 21.1%。
* **OS 徹底解耦與 Host 端狀態注入：** 從 App 層全面拔除 `#include "FreeRTOS.h"` 依賴。透過設定 `project.yml` 啟用 CMock `:return_thru_ptr` 外掛，在 Host 端精準注入假記憶體指標，達成降級運作模式 (Limp-Home) 觸發邏輯的 100% 程式碼覆蓋率。
* **MISRA C:2012 嚴格合規：** 依據 Tier-1 靜態分析標準，修復陣列隱式初始化、強制分支大括號與巨集防護 (Rules 9.3, 15.6, 20.9)，達成 CI/CD 流水線 100% 零違規綠燈通過。
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
│   │    ├── app_ssd1306.h
│   │    └── CMakeLists.txt   
│   ├── srv/                    # 服務層 (中介封裝)
│   │    ├── srv_os.h           # OS 抽象層介面 (解耦 FreeRTOS API)
│   │    ├── srv_os.h           # OS 抽象層介面 (解耦 FreeRTOS API)
│   │    ├── srv_fs.c           
│   │    ├── CMakeLists.txt
│   │    ├── config/                   
│   │    │    └── FreeRTOSConfig.h   
│   │    └── third_party/              # 第三方套件
│   │         ├── FreeRTOS-Kernel/     # FreeRTOS V11 核心源碼 (ARM_CM33 Port)
│   │         └── littlefs/
│   ├── utils/                  # 純邏輯基礎設施 (Data Structures)
│   │    ├── ring_buffer.c      # Lock-free 環形緩衝區實作
│   │    ├── ring_buffer.h
│   │    ├── observer.c         # MISRA C 規範事件派發中介軟體
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