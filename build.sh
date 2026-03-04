#!/bin/bash
set -e

BUILD_TYPE="Debug"
TARGET="firmware" 
DO_CLEAN=0        

IMAGE_NAME="project_forge_env:v2"
USER_ID=$(id -u)
GROUP_ID=$(id -g)

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -t|--test) TARGET="test" ;;
        -c|--clean) DO_CLEAN=1 ;;
        -r|--release) BUILD_TYPE="Release" ;;
        *) echo "Unknown parameter: $1"; exit 1 ;;
    esac
    shift
done

if [ $DO_CLEAN -eq 1 ]; then
    echo "🧹 Cleaning up build artifacts..."
    sudo rm -rf build build_test
    echo "✅ Clean complete."
fi

if [ "$TARGET" == "test" ]; then
    echo "🛡️ [Docker] Running Full QA Pipeline (Static Analysis + Unit Tests)..."
    
    # 💡 直接呼叫 CI 專用的 run_cppcheck.sh，確保本地與雲端 100% 一致！
    docker run --rm -v "$(pwd)":/project -w /project tier1-qa-env bash -c "
        bash tools/ci/run_cppcheck.sh && \
        echo '🧪 [2/2] Running Unit Tests via Ceedling...' && \
        ceedling clobber test:all && \
        chown -R ${USER_ID}:${GROUP_ID} build/test_build
    "
else
    echo "🔨 [Docker] Building Firmware for RP2350..."
    # 👇 這裡維持用原本的 project_forge_env:v2 來編譯硬體
    docker run --rm -v "$(pwd)":/workspace -w /workspace -e PICO_SDK_PATH=/opt/pico-sdk ${IMAGE_NAME} bash -c "
        mkdir -p build && cd build && \
        cmake -G Ninja -DCMAKE_BUILD_TYPE=${BUILD_TYPE} .. && \
        ninja && \
        cd .. && chown -R ${USER_ID}:${GROUP_ID} build
    "
fi