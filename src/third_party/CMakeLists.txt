# src/third_party/CMakeLists.txt

# 不要把代碼塞給 firmware，而是自己打包成一個獨立的圖書館 (Library)
add_library(littlefs_lib STATIC
    littlefs/lfs.c
    littlefs/lfs_util.c
)

# 告訴別人：想用我這個圖書館，要去哪裡找標頭檔
target_include_directories(littlefs_lib PUBLIC
    littlefs
)