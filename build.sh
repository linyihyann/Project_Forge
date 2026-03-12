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
    # 🌟 1. 拔除 sudo！因為以後檔案都是你的了
    rm -rf build build_test
    echo "✅ Clean complete."
fi

if [ "$TARGET" == "test" ]; then
    echo "🛡️ [Docker] Running Full QA Pipeline (Static Analysis + Unit Tests)..."
    
    # 🌟 2. 加上 --user，並移除結尾的 chown
    docker run --rm -v "$(pwd)":/project -w /project \
        --user ${USER_ID}:${GROUP_ID} \
        tier1-qa-env bash -c "
        bash tools/ci/run_cppcheck.sh && \
        echo '🧪 [2/2] Running Unit Tests via Ceedling...' && \
        ceedling clobber test:all
    "
else
    echo "🔨 [Docker] Building Firmware for RP2350..."
    
    # 🌟 3. 加上 --user，並移除結尾的 chown
    docker run --rm -v "$(pwd)":/workspace -w /workspace \
        -e PICO_SDK_PATH=/opt/pico-sdk \
        -e FREERTOS_KERNEL_PATH=/workspace/src/third_party/FreeRTOS-Kernel \
        --user ${USER_ID}:${GROUP_ID} \
        ${IMAGE_NAME} bash -c "
        mkdir -p build && cd build && \
        cmake -G Ninja -DCMAKE_BUILD_TYPE=${BUILD_TYPE} .. && \
        ninja
    "
fi