# 專門用於 MISRA C 靜態分析與 單元測試(TDD) 的全能 QA 無塵室
FROM ubuntu:22.04

ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8
ENV DEBIAN_FRONTEND=noninteractive

# 安裝 QA 必備工具：
# 1. cppcheck & python3 (MISRA 規則依賴)
# 2. ruby, gcc, make (Ceedling 單元測試環境依賴)
# (前面的部分保持不變)
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    cppcheck python3 \
    ruby gcc make libc6-dev && \
    gem install ceedling -v 0.31.1 && \
    rm -rf /var/lib/apt/lists/*

# (後面的部分保持不變)

WORKDIR /project