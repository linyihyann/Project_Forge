import serial
import os
import time

# 請替換成你 macOS 上的 Pico 2 W 序列埠名稱 (例如 /dev/cu.usbmodem14101)
PORT = '/dev/cu.usbmodemXXXX' 
BAUD = 921600
TEST_SIZE_MB = 1  # 測試 1MB 資料量

def run_stress_test():
    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
    except Exception as e:
        print(f"無法開啟 Port: {e}")
        return

    total_bytes = TEST_SIZE_MB * 1024 * 1024
    chunk_size = 1024 # 每次隨機丟 1024 bytes (非定長測試也可以改成 random)
    
    print(f"🚀 開始壓測 UART DMA Ping-Pong Buffer...")
    print(f"🎯 目標傳輸量: {TEST_SIZE_MB} MB @ {BAUD} bps")
    
    bytes_sent = 0
    bytes_received = 0
    start_time = time.time()

    while bytes_sent < total_bytes:
        # 產生隨機資料
        payload = os.urandom(chunk_size)
        
        # 發送
        ser.write(payload)
        bytes_sent += len(payload)
        
        # 接收 MCU 的 Loopback 回傳
        # 這裡會觸發 MCU 的 Timeout 中斷並切換 Buffer
        echo = ser.read(len(payload))
        bytes_received += len(echo)
        
        # 資料完整性校驗
        if payload != echo:
            print(f"\n❌ 測試失敗！資料不匹配 (Data Corruption) 或丟包！")
            print(f"進度: {bytes_sent}/{total_bytes} bytes")
            ser.close()
            return

        if bytes_sent % (100 * 1024) == 0:
            print(f"✅ 已成功收發 {bytes_sent // 1024} KB...")

    end_time = time.time()
    elapsed = end_time - start_time
    
    print("\n🎉 測試通過！(Tier-1 標準達成)")
    print(f"📊 總耗時: {elapsed:.2f} 秒")
    print(f"📊 實際吞吐量: {(total_bytes / elapsed / 1024):.2f} KB/s")
    print("🏆 丟包率: 0%, 資料損毀率: 0%")
    
    ser.close()

if __name__ == '__main__':
    run_stress_test()