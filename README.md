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

### 3. Immortal Build Infrastructure & Shift-Left QA Pipeline
* **Dual-Docker Architecture:** Separated hardware cross-compilation (`project_forge_env`) and host-based QA processes (`tier1-qa-env`) into dedicated containers, preventing environment drift and minimizing CI execution time.
* **Shift-Left Testing:** Implemented a unified `./build.sh -t` entry point and Git Pre-commit hooks to enforce MISRA C static analysis (Cppcheck) and Unit Tests (Ceedling) locally before any commit.
* **Bit-for-bit Reproducible:** Utilizes Multi-stage Docker builds setting deterministic epochs, guaranteeing exact firmware reproduction across any OS over the next decade.

---

## 🌟 核心軟體架構與亮點 (繁體中文版)

這是一個以 **Clean Architecture (乾淨架構)** 為核心設計的車規級嵌入式專案。具備 100% 跨平台移植能力、絕對可重製編譯環境 (Bit-for-bit Reproducibility) 與測試左移防禦工事 (Shift-Left Guardrails)。

### 1. TDD 測試驅動開發與 100% 覆蓋率 (Day 4 新增)
- **極速 Host 端驗證**：導入 `Ceedling / Unity / CMock` 工具鏈，將狀態機邏輯抽離硬體，在 macOS 上僅需 **0.5 秒** 即可完成全部極端邊界測試。
- **嚴格時序與 100% 分支覆蓋**：開啟 CMock 嚴格順序校驗 (Strict Ordering)，並透過 `gcov` 產出 HTML 覆蓋率報告，核心狀態機達成 **100% Branch Coverage**。

### 2. 車規級狀態機與 Fail-safe 安全防護 (Day 4 新增)
- **非阻塞超級迴圈 (Non-blocking)**：全面拔除阻塞式的 `delay()`，利用時間戳與 100Hz Tick 驅動狀態機，完美解決即時性 (Real-time) 瓶頸。
- **故障注入與安全降級**：實作 `SELF_TEST` 500ms 超時斷電防護，並具備底層硬體損壞的異常捕捉與 `FAULT` 狀態降級機制，符合防禦性編程 (Defensive Programming) 精神。

### 3. 左移測試與零誤差建置基礎設施 (Shift-Left QA Pipeline)
- **雙軌無塵室架構 (Dual-Docker)**：將 ARM Cortex-M33 交叉編譯與主機端 QA 測試徹底分離。使用獨立的 `tier1-qa-env` 執行 Ruby (Ceedling) 與 Cppcheck，杜絕環境污染並極大化 CI 效能。
- **左移測試 (Shift-Left Testing)**：透過 `build.sh -t` 一鍵觸發 MISRA C 靜態分析與單元測試。結合 Git Pre-commit Hook，在本地端強制執行排版與邏輯驗證，達成快速失敗 (Fail-Fast)。
- **100% 跨平台重現**：採用 Multi-stage Docker 封裝 Toolchain，保障未來 10 年內皆可 100% 重現當年的出廠韌體。

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