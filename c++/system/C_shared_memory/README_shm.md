## POSIX 共享内存读写示例

包含三个文件：
- `shm_common.h`：共享名称与数据结构定义
- `shm_writer.cpp`：写进程（创建并写入共享内存）
- `shm_reader.cpp`：读进程（打开并读取共享内存）

### 构建

```bash
g++ -std=c++17 -pthread c++/system/shm_writer.cpp -o /tmp/shm_writer -lrt
g++ -std=c++17 -pthread c++/system/shm_reader.cpp -o /tmp/shm_reader -lrt
```

or

```bash
mkdir build; cd build; cmake ..; make
```

说明：
- 一些发行版上 `-lrt` 可能可省略（glibc 新版把 `librt` 并入 libc），若链接失败再加。

### 运行

开两个终端执行：

终端1（先启动 writer，负责创建与 unlink 资源）：
```bash
/tmp/shm_writer
```

终端2（再启动 reader）：
```bash
/tmp/shm_reader
```

writer 会依次写入几条消息，最后发送 `END` 作为结束标志；reader 收到 `END` 后退出。writer 捕获 Ctrl+C 会清理共享对象。

### 通讯原理

- **数据通道：** 使用 POSIX 共享内存对象 `shm_open` + `mmap` 映射同一段物理内存，两个进程对同一 `SharedData` 结构体读写。
- **同步机制：** 使用命名信号量 `sem_open`：
  - `SEM_EMPTY` 初值 1，表示缓冲区空，可写。
  - `SEM_FULL` 初值 0，表示无数据，不可读。
  - writer：`sem_wait(EMPTY)` -> 写入 -> `sem_post(FULL)`。
  - reader：`sem_wait(FULL)` -> 读取 -> `sem_post(EMPTY)`。
- **生命周期：** writer 创建并在退出时 `shm_unlink/sem_unlink`，使资源在所有进程关闭后自动删除；reader 仅 `open/close` 不 `unlink`。

### 常见问题

- 若上次异常退出，残留的命名对象可能导致创建失败。可手工清理：
```bash
python3 - <<'PY'
import posixpath, os
for name in ['/shm_demo_v1','/shm_demo_empty_v1','/shm_demo_full_v1']:
    try:
        os.shm_unlink(name)
    except Exception:
        pass
PY
```


