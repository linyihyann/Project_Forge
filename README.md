# 🏎️ Project Forge (Tier-1 Embedded Architecture)

A production-ready, automotive-grade embedded C project utilizing **Clean Architecture**. 
Currently targeted for the **Raspberry Pi Pico 2 W (RP2350 / ARM Cortex-M33)**, the core logic is 100% platform-agnostic. This project features bit-for-bit reproducible builds, Test-Driven Development (TDD) guardrails, and automated memory profiling.

## 🌟 Key Features (English)

### 1. Test-Driven Development (TDD) & 100% Code Coverage
* **Host-based Testing:** Unit tests run on the host machine (macOS/Linux) using **Unity** and **CMock** via the **Ceedling** build system, executing in `< 0.5s` for instant feedback.
* **Strict Hardware Mocking:** CMock is configured with `:enforce_strict_ordering: TRUE` to mathematically guarantee the exact sequence of hardware API calls.
* **Gcov Integration:** Achieved **100% Branch Coverage**. Uncovered lines are strictly documented as MISRA C defensive programming (Unreachable Code) for ultimate reliability.

### 2. Deterministic State Machine (FSM) & Fail-safe Design
* **Non-blocking Super Loop:** Eradicated all `delay()` functions. Driven by a 100Hz periodic tick, liberating CPU cycles and preventing starvation of the wireless coprocessor (CYW43).
* **Fail-safe Mechanisms:** The FSM includes a strict 500ms timeout during the `SELF_TEST` state. Any hardware initialization failure or timeout immediately forces the system into a safe `FAULT` state.

### 3. Immortal Build Infrastructure & CI Guardrails
* **Bit-for-bit Reproducibility:** Uses a Multi-stage Docker clean room, `-fmacro-prefix-map`, and `SOURCE_DATE_EPOCH` to ensure identical binary hashes regardless of the host machine.
* **Shift-Left Pre-commit Hooks:** Custom Python scripts parse ELF `.bss` and `.data` sections locally. Commits are automatically rejected if RAM usage spikes `> 5KB`, preventing memory leaks at the source.

---

## 🌟 核心軟體架構與亮點 (繁體中文版)

這是一個以 **Clean Architecture (乾淨架構)** 為核心設計的車規級嵌入式專案。具備 100% 跨平台移植能力、絕對可重製編譯環境 (Bit-for-bit Reproducibility) 與測試左移防禦工事 (Shift-Left Guardrails)。

### 1. TDD 測試驅動開發與 100% 覆蓋率 (Day 4 新增)
- **極速 Host 端驗證**：導入 `Ceedling / Unity / CMock` 工具鏈，將狀態機邏輯抽離硬體，在 macOS 上僅需 **0.5 秒** 即可完成全部極端邊界測試。
- **嚴格時序與 100% 分支覆蓋**：開啟 CMock 嚴格順序校驗 (Strict Ordering)，並透過 `gcov` 產出 HTML 覆蓋率報告，核心狀態機達成 **100% Branch Coverage**。

### 2. 車規級狀態機與 Fail-safe 安全防護 (Day 4 新增)
- **非阻塞超級迴圈 (Non-blocking)**：全面拔除阻塞式的 `delay()`，利用時間戳與 100Hz Tick 驅動狀態機，完美解決即時性 (Real-time) 瓶頸。
- **故障注入與安全降級**：實作 `SELF_TEST` 500ms 超時斷電防護，並具備底層硬體損壞的異常捕捉與 `FAULT` 狀態降級機制，符合防禦性編程 (Defensive Programming) 精神。

### 3. 零誤差建置基礎設施 (Immortal Build)
- **100% 跨平台無塵室**：採用 Multi-stage Docker 封裝 ARM Toolchain，保障未來 10 年內皆可 100% 重現當年的出廠韌體。
- **記憶體暴增防禦網**：自研 Git Pre-commit Hook (`mem_profiler.py`)，於開發者本地端攔截巨型靜態陣列與 Memory Leak 隱患。
- **現代化 CMake 裝配**：採用 `target_sources` 扁平化管理，避開 ARM Cortex-M33 靜態庫打包常見的危險跳躍 (Dangerous Relocation) 異常。

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
│   └── support/
├── src/
│   ├── app/              # 跨平台業務邏輯層 (100% 獨立)
│   │    ├── app_fsm.c    # 核心狀態機 (Self-test, Heartbeat, Fail-safe)
│   │    ├── app_main.c   # 系統主任務 (Non-blocking Super Loop)      
│   │    ├── app_main.h    
│   │    └── CMakeLists.txt   
│   └── hal/              # 硬體抽象層 (Hardware Abstraction Layer)
│       ├── include/      # HAL 介面定義 (統一回傳 Status Code)
│       │    ├── hal_dio.h      
│       │    └── hal_time.h     
│       └── rp2350/       # RP2350 實體驅動實作 (CYW43 LED, Timer)
│            ├── hal_dio_rp2350.c    
│            ├── hal_time_rp2350.c  
│            └── CMakeLists.txt 