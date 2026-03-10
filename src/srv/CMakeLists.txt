# ==========================================
# src/srv/CMakeLists.txt
# 負責管理「服務層 (Service Layer)」的原始碼
# ==========================================

# 將我們 Day 10 寫的 LittleFS 靜態封裝層，掛載給 firmware 廠長
target_sources(firmware PRIVATE
    srv_fs.c
)