#!/bin/bash
set -e

BUILD_TYPE="Debug"
TARGET="firmware" 
DO_CLEAN=0        

# 🌟 強制升版到 v2，徹底斷開與舊版 v1 的糾纏！
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
    echo "🧪 [Docker] Building and Running Unit Tests..."
    docker run --rm -v "$(pwd)":/workspace -w /workspace -e PICO_SDK_PATH=/opt/pico-sdk ${IMAGE_NAME} bash -c "
        mkdir -p build_test && cd build_test && \
        cmake -G Ninja -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ../test && \
        ninja && \
        ./run_tests && \
        cd .. && chown -R ${USER_ID}:${GROUP_ID} build_test
    "
else
    echo "🔨 [Docker] Building Firmware for RP2350..."
    docker run --rm -v "$(pwd)":/workspace -w /workspace -e PICO_SDK_PATH=/opt/pico-sdk ${IMAGE_NAME} bash -c "
        mkdir -p build && cd build && \
        cmake -G Ninja -DCMAKE_BUILD_TYPE=${BUILD_TYPE} .. && \
        ninja && \
        cd .. && chown -R ${USER_ID}:${GROUP_ID} build
    "
    echo "✅ Firmware built successfully via Docker!"
fi