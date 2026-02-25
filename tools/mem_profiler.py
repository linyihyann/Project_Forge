import os
import sys
import subprocess
import json
from datetime import datetime

# 配置區
ELF_PATH = "build/firmware.elf"
SIZE_CMD = "arm-none-eabi-size"
JSON_PATH = "tools/mem_baseline.json"
THRESHOLD_BYTES = 5120  # 5KB 門檻

def run_size_tool(elf_path):
    """呼叫 Toolchain 解析 ELF"""
    if not os.path.exists(elf_path):
        print(f"❌ [Error] 找不到 {elf_path}！請先進行 Build (Docker)。")
        sys.exit(1)
    
    result = subprocess.run([SIZE_CMD, elf_path], capture_output=True, text=True)
    if result.returncode != 0:
        print(f"❌ [Error] size 指令執行失敗:\n{result.stderr}")
        sys.exit(1)
    return result.stdout

def parse_size_output(output_text):
    """解析字串提取 data 與 bss 大小"""
    lines = output_text.strip().split('\n')
    if len(lines) < 2:
        sys.exit(1)
    
    # 取第二行資料: text, data, bss, dec, hex, filename
    parts = lines[1].split() 
    return {"data_size": int(parts[1]), "bss_size": int(parts[2])}

def main():
    print("🔍 [Memory Profiler] 正在分析 RAM 消耗...")
    
    # 1. 取得當前 Size
    size_output = run_size_tool(ELF_PATH)
    current_mem = parse_size_output(size_output)
    current_ram = current_mem["data_size"] + current_mem["bss_size"]
    
    # 2. 讀取 Baseline
    try:
        with open(JSON_PATH, 'r') as f:
            baseline = json.load(f)
    except Exception:
        baseline = {"bss_size": 0, "data_size": 0}
        
    baseline_ram = baseline.get("data_size", 0) + baseline.get("bss_size", 0)
    
    # 3. 計算增量與防禦判定
    delta = current_ram - baseline_ram
    print(f"📊 歷史 RAM: {baseline_ram} bytes | 當前 RAM: {current_ram} bytes | 增量: {delta} bytes")
    
    if delta > THRESHOLD_BYTES:
        print(f"🚨 [Git REJECT] 記憶體暴增防禦觸發！")
        print(f"❌ 單次 Commit RAM 增加 {delta} bytes，超過 {THRESHOLD_BYTES} bytes 的安全門檻。")
        print(f"💡 請檢查是否宣告了過大的全域/靜態陣列，或忘記加上 const。")
        sys.exit(1) # Git 會因為這個非零退出碼而擋下 Commit
        
    # 4. 更新 Baseline (若通過)
    current_mem["last_update"] = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    with open(JSON_PATH, 'w') as f:
        json.dump(current_mem, f, indent=4)
        
    print("✅ [Memory Profiler] 檢查通過！")
    sys.exit(0)

if __name__ == "__main__":
    main()