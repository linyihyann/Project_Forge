# 🏎️ Project Forge (Tier-1 Embedded Architecture)

這是一個以 **Clean Architecture (乾淨架構)** 為核心設計的車規級嵌入式 C 語言專案。
目前硬體平台運行於 **Raspberry Pi Pico 2 W (RP2350 / ARM Cortex-M33)**，但其核心演算法具備 100% 的跨平台移植能力。

## 🌟 核心架構與亮點 (Key Features)

1. **絕對解耦 (Separation of Concerns)**
   - `src/app` 業務邏輯層 0% 依賴硬體標頭檔，可無痛移植至任何 MCU (如 STM32, NXP, ESP32)。
   - 採用 **單向依賴** 原則：App 僅依賴 HAL 介面，硬體細節完全封裝於 `src/hal/rp2350`。

2. **非阻塞超級迴圈 (Non-blocking Super Loop)**
   - 捨棄傳統的 `delay()` 阻斷式寫法。
   - 使用 `100Hz 系統心跳` 與軟體計數器實現精確排程，完美解決無線通訊協處理器 (CYW43) 的 SPI 通訊飢餓 (Starvation) 問題。

3. **現代化構建系統防禦 (Modern CMake)**
   - 解決了 ARM Cortex-M33 於靜態庫打包時常見的 `dangerous relocation` (危險跳躍) 異常。
   - 透過 `target_sources` 實現精準的源碼掛載，避開 SDK 雙重連結引發的 `multiple definition`。

## 🛡️ 架構防護網測試 (Architecture FMEA & Testing)
本專案已通過以下架構級稽核：
- [x] **跨平台編譯測試**: 將目標轉為 macOS native (Host 端 clang)，App 層代碼達 `0 Error, 0 Warning`。
- [x] **依賴阻擋測試**: 於 App 層刻意 `#include <hardware/gpio.h>`，CMake 立即攔截並報錯。
- [x] **無阻斷與無暫存器操作**: App 模組內經 Regex 掃描，包含 `0` 筆 `sleep` 呼叫與 `0` 筆 `volatile` 暫存器指標操作。

## 📂 目錄結構
```text
├── CMakeLists.txt        # 總裝配線 (Composition Root)
├── main.c                # 乾淨的系統進入點
├── src/
│   ├── app/              # 跨平台業務邏輯層 (100% 獨立)
│   └── hal/              # 硬體抽象層
│       ├── include/      # HAL 介面定義 (無硬體相依)
│       └── rp2350/       # RP2350 實體驅動實作