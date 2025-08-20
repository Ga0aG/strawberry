""" process_print.py
import time
for i in range(5):
    if i % 2 == 0:
        print(f"{i} is even")
    else:
        print(f"{i} is odd")
    time.sleep(1)
"""


import subprocess
# -u 无缓冲模式，确保输出实时显示
process = subprocess.Popen(["python3", "-u", "process_print.py"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

while True:
    line = process.stdout.readline()
    if line:
        print(line.rstrip().decode('utf-8'))
    else:
        # 检查进程是否已经结束
        if process.poll() is not None:
            break
# 等待进程完全结束并获取退出码
exit_code = process.wait()
print(f"Process exited with code: {exit_code}")
