#!/bin/bash
# 腳本：嚴格模式 MISRA C 靜態分析 (Local & CI 共用)

echo "🔍 啟動 Tier-1 級別 MISRA C 靜態分析..."

cd "$(dirname "$0")/../../" || exit 1

# 新增了 -I (大寫i) 讓工具找得到標頭檔
# 新增了 suppress 15.5 (單一 return) 與 8.7 (外部連結)
cppcheck --enable=all \
         --addon=tools/ci/misra.json \
         --inline-suppr \
         --error-exitcode=1 \
         --suppress=missingIncludeSystem \
         --suppress=unusedFunction \
         --suppress=misra-c2012-15.5 \
         --suppress=misra-c2012-8.7 \
         --suppress=unmatchedSuppression \
         -I src/app \
         -I src/hal/rp2350/ \
         -I src/hal/include/ \
         -I test \
         src/app

EXIT_CODE=$?

if [ $EXIT_CODE -eq 0 ]; then
    echo "✅ MISRA C 檢查通過！(src/app 乾淨無瑕)"
else
    echo "❌ 發現違規！請修正上述 MISRA C / 靜態分析錯誤。"
    exit 1
fi