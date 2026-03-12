cmake_minimum_required(VERSION 3.13)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(PICO_PLATFORM rp2350-arm-s CACHE STRING "Target platform")
set(PICO_BOARD pico2_w CACHE STRING "Target board")

include(pico_sdk_import.cmake)
project(Project_Forge C CXX ASM)
pico_sdk_init()

add_compile_options(
    "-fmacro-prefix-map=${CMAKE_SOURCE_DIR}=."
    "-fdebug-prefix-map=${CMAKE_SOURCE_DIR}=."
)

add_executable(firmware main.c)

add_subdirectory(src/app)
add_subdirectory(src/utils)
add_subdirectory(src/hal/rp2350)
add_subdirectory(src/srv)
add_subdirectory(src/srv/third_party)

set(FREERTOS_SRC_DIR "src/srv/third_party/FreeRTOS-Kernel")
set(FREERTOS_PORT_DIR "${FREERTOS_SRC_DIR}/portable/GCC/ARM_CM4F")

target_sources(firmware PRIVATE
    ${FREERTOS_SRC_DIR}/tasks.c
    ${FREERTOS_SRC_DIR}/queue.c
    ${FREERTOS_SRC_DIR}/list.c
    ${FREERTOS_SRC_DIR}/timers.c
    ${FREERTOS_SRC_DIR}/event_groups.c
    ${FREERTOS_SRC_DIR}/stream_buffer.c
    ${FREERTOS_PORT_DIR}/port.c
    ${FREERTOS_SRC_DIR}/portable/MemMang/heap_4.c
)

target_include_directories(firmware PRIVATE
    src/third_party/littlefs  
    src/srv                   
    src/hal/include           
    src/srv/config
    ${FREERTOS_SRC_DIR}/include
    ${FREERTOS_PORT_DIR}
)

target_link_libraries(firmware PRIVATE 
    pico_standard_link
    pico_cyw43_arch_none
    pico_stdlib
    pico_stdio_usb
    littlefs_lib
    hardware_i2c      
    hardware_dma      
    hardware_uart     
    hardware_watchdog 
    pico_multicore        
)

target_compile_definitions(firmware PRIVATE CYW43_LWIP=0)

pico_enable_stdio_usb(firmware 1)  
pico_enable_stdio_uart(firmware 0)

pico_add_extra_outputs(firmware)