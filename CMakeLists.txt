cmake_minimum_required(VERSION 3.13)

set(PICO_PLATFORM rp2350-arm-s CACHE STRING "Target platform")
set(PICO_BOARD pico2_w CACHE STRING "Target board")

include(pico_sdk_import.cmake)
project(Project_Forge C CXX ASM)
pico_sdk_init()

# 1. 🌟 先創造執行檔 (廠長)
add_executable(firmware main.c)

# 2. 🌟 呼叫子資料夾 (它們會把代碼直接掛載給 firmware)
add_subdirectory(src/app)
add_subdirectory(src/hal/rp2350)

# 3. 🌟 廠長統一配發引擎與底層 (只在這裡呼叫一次 SDK)
target_link_libraries(firmware PRIVATE 
    pico_standard_link
    pico_cyw43_arch_none 
    pico_stdlib
)

pico_add_extra_outputs(firmware)