#!/bin/bash
# 腳本：Docker 環境專用轉發器
# 🌟 單一事實來源 (SSoT)：不在此處維護邏輯，直接執行核心檢查腳本

# 取得腳本所在的資料夾路徑，並執行同目錄下的 run_cppcheck.sh
SCRIPT_DIR="$(dirname "$0")"
bash "${SCRIPT_DIR}/run_cppcheck.sh"