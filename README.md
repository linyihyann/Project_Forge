# 🏎️ Project Forge (Tier-1 Embedded Architecture)

這是一個以 **Clean Architecture (乾淨架構)** 為核心設計的車規級嵌入式 C 語言專案。
目前硬體平台運行於 **Raspberry Pi Pico 2 W (RP2350 / ARM Cortex-M33)**，但其核心演算法具備 100% 的跨平台移植能力。本專案同時具備車廠級別的**絕對可重製編譯環境 (Bit-for-bit Reproducibility)** 與 **測試左移防禦工事 (Shift-Left Guardrails)**。

## 🌟 核心軟體架構 (Software Architecture)

1. **絕對解耦 (Separation of Concerns)**
   - `src/app` 業務邏輯層 0% 依賴硬體標頭檔，可無痛移植至任何 MCU (如 STM32, NXP, ESP32)。
   - 採用 **單向依賴** 原則：App 僅依賴 HAL 介面，硬體細節完全封裝於 `src/hal/rp2350`。

2. **非阻塞超級迴圈 (Non-blocking Super Loop)**
   - 捨棄傳統的 `delay()` 阻斷式寫法。
   - 使用 `100Hz 系統心跳` 與軟體計數器實現精確排程，完美解決無線通訊協處理器 (CYW43) 的 SPI 通訊飢餓 (Starvation) 問題。

3. **現代化構建系統防禦 (Modern CMake)**
   - 解決了 ARM Cortex-M33 於靜態庫打包時常見的 `dangerous relocation` (危險跳躍) 異常。
   - 透過 `target_sources` 實現精準的源碼掛載，避開 SDK 雙重連結引發的 `multiple definition`。

## 🏗️ 零誤差建置基礎設施 (Immortal Build Infrastructure)

本專案導入了 Tier-1 車廠的 CI/CD 底層防禦，徹底消滅 "Works on my machine" 的量產災難：

1. **🐳 100% 跨平台無塵室 (Multi-stage Docker)**
   - **Stage 1 (Fetcher):** 隔離網路依賴，預先下載 ARM Toolchain 並編譯 `picotool`。
   - **Stage 2 (Clean Room):** 純淨的離線編譯環境，保障未來 5 年或 10 年後，即使 GitHub 倒閉也能 100% 重現當年的出廠韌體。

2. **⚖️ 絕對二進制一致性 (Bit-for-bit Reproducibility)**
   - 注入 `SOURCE_DATE_EPOCH` 抹除 GCC 隱藏的時間戳變數 (`__DATE__`, `__TIME__`)。
   - 透過 CMake `-fmacro-prefix-map` 解耦開發機絕對路徑，雙平台 (macOS/Linux) 編譯產出的 Hash 碼 100% 絕對一致。

3. **⚡ 極速編譯與權限自適應 (VirtioFS Permission Mapping)**
   - 解決 macOS Docker 跨平台常見的權限鎖死陷阱，封裝 `build.sh` 實現容器內 root 編譯、結束後自動 `chown` 交還 Host 權限。
   - **效能突破**：M1 Mac 透過原生 ARM64 容器搭配 Ninja，Clean Build (全編譯) 耗時從 8 分鐘 (Rosetta) 壓縮至 **~6 秒**。

## 🛡️ 測試左移與防禦工事 (Shift-Left Testing & Guardrails)

為確保量產極致可靠度，本專案將防呆機制「左移」至開發者的本機端，透過 `git config core.hooksPath` 實現基礎設施化 (Infrastructure as Code) 的 Git Pre-commit 攔截網：

1. **🧠 記憶體暴增防禦 (Automated Memory Profiling)**
   - 內建 Python 腳本 (`mem_profiler.py`)，於 Commit 時自動編譯並解析 ELF 檔的 `.bss` 與 `.data` 區段。
   - 結合本地端 JSON 歷史基準點，當單次 Commit 的 RAM 增量 **> 5KB** 時，精準觸發 Git Reject，從源頭掐死巨型靜態陣列與 Memory Leak 隱患。
2. **📏 絕對代碼紀律 (Strict Code Discipline)**
   - **排版防禦**：全面掛載 `.clang-format` (Google/Allman 風格)，強制統一團隊排版，消滅視覺邏輯漏洞。
   - **靜態分析**：自動觸發 `cppcheck`，嚴格執行 MISRA C Level 1 檢查，確保不合規代碼無法污染主線。
3. **⏱️ 零痛感開發者體驗 (Zero-cost DX)**
   - 上述 4 道防線檢測耗時控制在 **< 2 秒** 內，兼顧車規級嚴謹與敏捷開發速度。

## 📋 架構防護網測試 (Architecture FMEA & Testing)
本專案已通過以下架構級別稽核：
- [x] **二進制可重製測試**: 執行連續兩次 `./build.sh -c`，產出之 `.uf2` Hash 碼 (`shasum -a 256`) 100% 一致。
- [x] **依賴阻擋測試**: 於 App 層刻意 `#include <hardware/gpio.h>`，CMake 立即攔截並報錯。
- [x] **無阻斷與無暫存器操作**: App 模組內經 Regex 掃描，包含 `0` 筆 `sleep` 呼叫與 `0` 筆 `volatile` 暫存器指標操作。
- [x] **記憶體攔截測試**: 刻意宣告 `6000 bytes` 之全域變數並進行讀寫，Pre-commit Hook 成功於 2 秒內阻擋提交。

## 🚀 快速啟動 (Quick Start)

**1. 初始化建置環境與防禦網 (僅需執行一次)**
```bash
# 建立編譯環境
docker build -t project_forge_env:v2 .

# 啟動 Git 架構防禦網 (攔截不合規 Commit)
git config core.hooksPath tools/git-hooks

## 📂 目錄結構
```text
├── CMakeLists.txt        # 總裝配線 (Composition Root)
├── Dockerfile            # 跨平台無塵室構建圖紙 (Multi-stage)
├── build.sh              # 自動化編譯與跨平台權限封裝腳本
├── main.c                # 乾淨的系統進入點
├── .clang-format         # 車規級代碼排版規範 (Allman Style)
├── tools/                # 基礎設施與防禦工具
│   ├── git-hooks/        # 統一管理的 Git Hooks
│   ├── mem_profiler.py   # ELF 記憶體解析與攔截腳本
│   └── mem_baseline.json # 記憶體用量歷史基準點
├── src/
│   ├── app/              # 跨平台業務邏輯層 (100% 獨立)
│   └── hal/              # 硬體抽象層
│       ├── include/      # HAL 介面定義 (無硬體相依)
│       └── rp2350/       # RP2350 實體驅動實作