# ==========================================
# Stage 1: Fetcher & Builder (獲取與預先編譯層)
# ==========================================
FROM ubuntu:22.04 AS fetcher
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    wget ca-certificates xz-utils git build-essential cmake pkg-config libusb-1.0-0-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /tmp_build

# 1. 下載 ARM Toolchain (AArch64)
ENV TOOLCHAIN_URL="https://developer.arm.com/-/media/Files/downloads/gnu/13.3.rel1/binrel/arm-gnu-toolchain-13.3.rel1-aarch64-arm-none-eabi.tar.xz"
RUN wget -q ${TOOLCHAIN_URL} -O toolchain.tar.xz && \
    mkdir -p /opt/toolchain && \
    tar -xf toolchain.tar.xz -C /opt/toolchain --strip-components=1 && \
    rm toolchain.tar.xz

# 2. 下載 Pico SDK
RUN git clone --depth=1 --branch 2.1.0 https://github.com/raspberrypi/pico-sdk.git /opt/pico-sdk && \
    cd /opt/pico-sdk && git submodule update --init && \
    find . -name ".git" -type d -exec rm -rf {} +

# 3. 預先編譯並安裝 Picotool
ENV PICO_SDK_PATH=/opt/pico-sdk
RUN git clone --depth=1 --branch 2.1.0 https://github.com/raspberrypi/picotool.git /tmp_build/picotool && \
    cd /tmp_build/picotool && mkdir build && cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=/opt/picotool_install .. && \
    make && make install

# ==========================================
# Stage 2: Final Build Environment (無塵室編譯層)
# ==========================================
FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake ninja-build python3 build-essential xxd libnewlib-arm-none-eabi libusb-1.0-0 \
    && rm -rf /var/lib/apt/lists/* && apt-get clean

# 🌟 從 Stage 1 拷貝所有精華到 Stage 2
COPY --from=fetcher /opt/toolchain /opt/toolchain
COPY --from=fetcher /opt/pico-sdk /opt/pico-sdk
COPY --from=fetcher /opt/picotool_install /usr/local/

ENV PATH="/opt/toolchain/bin:$PATH"
ENV PICO_SDK_PATH=/opt/pico-sdk
ENV PICO_SDK_FETCH_FROM_GIT_PATH=0
ENV PICO_SDK_NO_VERSION_CHECK=1
ENV SOURCE_DATE_EPOCH=1704067200

WORKDIR /workspace
CMD ["/bin/bash"]