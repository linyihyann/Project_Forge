# 將 ring_buffer.c 掛載到主程式
target_sources(firmware PRIVATE 
    ring_buffer.c
    observer.c
)

# 讓 firmware 知道去哪裡找 ring_buffer.h
target_include_directories(firmware PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})