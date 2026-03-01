# 專門用於 MISRA C 與靜態分析的輕量化品保無塵室
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# 只安裝 QA 必備工具：cppcheck 與 python3 (MISRA 規則依賴)
RUN apt-get update && \
    apt-get install -y --no-install-recommends cppcheck python3 && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /project