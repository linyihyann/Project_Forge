# ==========================================
# src/srv/CMakeLists.txt
# 負責管理「服務層 (Service Layer)」的原始碼
# ==========================================

target_sources(firmware PRIVATE
    srv_fs.c
    srv_os.c
    config/FreeRTOSConfig.h
)