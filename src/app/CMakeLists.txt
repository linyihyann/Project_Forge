# 🌟 不再打包成獨立庫，直接把源碼附加給廠長 (firmware)
target_sources(firmware PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app_main.c)

# 🌟 設定大腦需要的標頭檔路徑
target_include_directories(firmware PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../hal/include
)