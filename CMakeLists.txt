cmake_minimum_required(VERSION 3.13)

# ==============================================================================
# 🛡️ [架構師防禦 1] 裸機交叉編譯設定 (解決 undefined reference to _exit)
# 必須放在 project() 宣告之前！強制 CMake 體檢時只做編譯、不連結。
# ==============================================================================
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# 設定 Pico 2 W 的目標平台與板子
set(PICO_PLATFORM rp2350-arm-s CACHE STRING "Target platform")
set(PICO_BOARD pico2_w CACHE STRING "Target board")

include(pico_sdk_import.cmake)
project(Project_Forge C CXX ASM)
pico_sdk_init()

# ==============================================================================
# 🛡️ [架構師防禦 2] 消滅絕對路徑，確保 Day 2 的跨機器二進制 100% 一致性
# ==============================================================================
add_compile_options(
    "-fmacro-prefix-map=${CMAKE_SOURCE_DIR}=."
    "-fdebug-prefix-map=${CMAKE_SOURCE_DIR}=."
)

# 1. 🌟 先創造執行檔 (廠長)
add_executable(firmware main.c)

# 2. 🌟 呼叫子資料夾 (它們會把代碼直接掛載給 firmware)
add_subdirectory(src/app)
add_subdirectory(src/utils)
add_subdirectory(src/hal/rp2350)

# 3. 🌟 廠長統一配發引擎與底層 (只在這裡呼叫一次 SDK)
target_link_libraries(firmware PRIVATE 
    pico_standard_link
    pico_cyw43_arch_none 
    pico_stdlib
    hardware_i2c      # 🌟 加上這行！賦予 firmware 使用 I2C 暫存器的權限
    hardware_dma      # (如果你之前 DMA 沒加，建議一併補上)
    hardware_uart     # (如果你之前 UART 沒加，建議一併補上)   
)

pico_enable_stdio_usb(firmware 1)
pico_enable_stdio_uart(firmware 0)

pico_add_extra_outputs(firmware)