# 🌟 直接把翻譯官源碼附加給廠長 (firmware)
target_sources(firmware PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/hal_dio_rp2350.c 
    ${CMAKE_CURRENT_SOURCE_DIR}/hal_time_rp2350.c
    ${CMAKE_CURRENT_SOURCE_DIR}/hal_dma_rp2350.c
    ${CMAKE_CURRENT_SOURCE_DIR}/hal_i2c_rp2350.c
     ${CMAKE_CURRENT_SOURCE_DIR}/hal_flash_rp2350.c
)

# 🌟 設定翻譯官需要的標頭檔路徑
target_include_directories(firmware PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)