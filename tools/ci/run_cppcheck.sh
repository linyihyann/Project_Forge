#!/bin/bash
# 腳本：嚴格模式 MISRA C 靜態分析 (Local & CI 共用)

echo "🔍 啟動 Tier-1 級別 MISRA C 靜態分析..."

cd "$(dirname "$0")/../../" || exit 1

cppcheck --enable=all \
         --addon=tools/ci/misra.json \
         --inline-suppr \
         --error-exitcode=1 \
         --suppress=missingIncludeSystem \
         --suppress=unusedFunction \
         --suppress=misra-c2012-15.5 \
         --suppress=misra-c2012-8.7 \
         --suppress=misra-c2012-10.4 \
         --suppress=misra-c2012-21.6 \
         --suppress=misra-c2012-17.3 \
         --suppress=misra-c2012-18.8 \
         --suppress=misra-c2012-11.8 \
         --suppress=missingInclude \
         --suppress=unmatchedSuppression \
         --suppress=toomanyconfigs \
         --suppress=normalCheckLevelMaxBranches \
         --suppress=misra-config \
         --suppress=constParameterPointer \
         --suppress=*:*src/srv/third_party/* \
         -i src/srv/third_party/ \
         -I src/app \
         -I src/hal/rp2350/ \
         -I src/hal/include/ \
         -I src/utils/ \
         -I src/srv/ \
         -I src/srv/third_party/littlefs/ \
         -I test \
         main.c \
         src/app \
         src/utils \
         src/srv

EXIT_CODE=$?

if [ $EXIT_CODE -eq 0 ]; then
    echo "✅ MISRA C 檢查通過！"
else
    echo "❌ 發現違規！請修正上述 MISRA C / 靜態分析錯誤。"
    exit 1
fi