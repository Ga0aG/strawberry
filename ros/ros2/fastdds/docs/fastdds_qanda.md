代码可以参考eProsima/Fast-DDS下的代码
不要修改“问题描述”，只要在“问题回答”里面更新就行，并在不断的更新中不要让文档变得混乱，要保持逻辑层级的清晰可读。


# 问题描述

系统的网络拓扑图：

```mermaid
flowchart TD
    subgraph R1[单个机器人 #1]
        direction LR
        SW_A1[内部交换机 A1<br/>非网管型<br/>网段: 192.168.0.0/24]

        SW_A1 -- 物理网线直连 --> ORIN1[Orin板卡<br/>eth0: 192.168.0.11<br/>ROS通信]
        SW_A1 -- 物理网线直连 --> X86_1[x86计算机<br/>IP: 192.168.0.10<br/>ROS通信]

    end

    ORIN1 -- 无线Wi-Fi连接 --> WIFI
    subgraph R2[单个机器人 #2]
        direction LR
        SW_A2[内部交换机 A2]
        SW_A2 --> ORIN2[Orin板卡<br/>eth0: 192.168.11]
        SW_A2 --> X86_2[x86计算机<br/>IP: 192.168.0.10]
    end
    ORIN2 -- 无线Wi-Fi --> WIFI
    RN_CONTENT -- 无线Wi-Fi --> WIFI

    subgraph RN[机器人 #N...]
        RN_CONTENT[......]
    end

    WIFI{公司局域网Wi-Fi}

    用户电脑[用户电脑<br/>手动设置IP: 192.168.0.100] -. 调试时临时连接 .-> SW_A1
    用户电脑 -. 调试时临时连接 .-> SW_A2
    用户电脑 .-> WIFI
```

传感器消息定义:

```bash
astribot@zhuojun-System-Product-Name:/opt/astribot_ros/meta/astribot_sdk$ ros2 topic info -v /astribot_camera/head_rgbd/color_compress/compressed
Type: sensor_msgs/msg/CompressedImage

Publisher count: 1

Node name: compress_camera_driver_node
Node namespace: /
Topic type: sensor_msgs/msg/CompressedImage
Endpoint type: PUBLISHER
GID: 01.0f.9a.ac.a2.6f.c4.a0.00.00.00.00.00.00.1c.03.00.00.00.00.00.00.00.00
QoS profile:
  Reliability: RELIABLE
  History (Depth): UNKNOWN
  Durability: VOLATILE
  Lifespan: Infinite
  Deadline: Infinite
  Liveliness: AUTOMATIC
  Liveliness lease duration: Infinite

Subscription count: 0

astribot@zhuojun-System-Product-Name:/opt/astribot_ros/meta/astribot_sdk$ ros2 topic info /livox/lidar_front -v
Type: sensor_msgs/msg/PointCloud2

Publisher count: 1

Node name: livox_lidar_publisher
Node namespace: /
Topic type: sensor_msgs/msg/PointCloud2
Endpoint type: PUBLISHER
GID: 01.0f.9a.ac.0c.30.86.6d.00.00.00.00.00.00.13.03.00.00.00.00.00.00.00.00
QoS profile:
  Reliability: BEST_EFFORT
  History (Depth): UNKNOWN
  Durability: VOLATILE
  Lifespan: Infinite
  Deadline: Infinite
  Liveliness: AUTOMATIC
  Liveliness lease duration: Infinite

Subscription count: 0
```

fastdds的配置（只有orin和用户电脑配置了，分别是它们的IP）
```xml
<?xml version="1.0" encoding="UTF-8"?>
<dds>
    <profiles xmlns="http://www.eprosima.com/XMLSchemas/fastRTPS_Profiles">
        <transport_descriptors>
            <transport_descriptor>
                <transport_id>UDPv4Transport</transport_id>
                <type>UDPv4</type>
                <interfaceWhiteList>
                    <address>192.168.0.100</address>
                    <address>127.0.0.1</address> <!-- fix: 让机器能听到自己发出来的消息 -->
                </interfaceWhiteList>
		<sendBufferSize>16777216</sendBufferSize>      <!-- 16MB -->
		<receiveBufferSize>16777216</receiveBufferSize> <!-- 16MB -->
            </transport_descriptor>
        </transport_descriptors>
        <participant profile_name="default_participant" is_default_profile="true">
            <rtps>
                <userTransports>
                    <transport_id>UDPv4Transport</transport_id>
                </userTransports>
                <useBuiltinTransports>false</useBuiltinTransports>
            </rtps>
        </participant>
    </profiles>
</dds>
```

用户电脑上配置了fastdds的配置后，有的电脑和x86一样能正常收到雷达数据，有的容易丢数据：
```bash
astribot@zhuojun-System-Product-Name:/opt/astribot_ros/meta/astribot_sdk$ ros2 topic echo /livox/lidar_front --field header.stamp
sec: 1770704090
nanosec: 200066810
---
A message was lost!!!
    total count change:1
    total count: 1---
sec: 1770704090
nanosec: 400226810
---
...
---
sec: 1770704106
nanosec: 800485418
---
sec: 1770704106
nanosec: 900325418
---
sec: 1770704107
nanosec: 165418
---
A message was lost!!!
    total count change:1
    total count: 17---
sec: 1770704107
nanosec: 200330802
---
sec: 1770704107
nanosec: 300170802
---
...
---
sec: 1770704109
nanosec: 500023082
---
A message was lost!!!
    total count change:112
    total count: 129---

```

orin上设置了防火墙
```
sudo iptables -I INPUT -i wlP1p1s0 -d 239.255.0.1 -j DROP
sudo iptables -I OUTPUT -o wlP1p1s0 -d 239.255.0.1 -j DROP
```

点云大小：

```python
import rclpy
from sensor_msgs.msg import PointCloud2
import sys
from rclpy.qos import QoSProfile, ReliabilityPolicy, HistoryPolicy


qos_profile = QoSProfile(
    reliability=ReliabilityPolicy.BEST_EFFORT,
    history=HistoryPolicy.KEEP_LAST,
    depth=10
)

length = 0
totalSize = 0

def callback(msg):
    # 计算消息大小
    global length
    global totalSize
    try:
        size_bytes = len(msg.data)
        size_mb = size_bytes / (1024 * 1024)
        totalSize += size_bytes

        print(f"PointCloud2 消息大小:")
        print(f"  原始数据: {size_bytes} 字节 ({size_mb:.2f} MB)")
        print(f"  点数: {msg.width * msg.height}")
        print(f"  宽度: {msg.width}, 高度: {msg.height}")
        print(f"  点步长: {msg.point_step} 字节/点")
        print(f"  行步长: {msg.row_step} 字节/行")
        print(f"  字段数: {len(msg.fields)}")
        for field in msg.fields:
            print(f"    - {field.name}: offset={field.offset}, datatype={field.datatype}")
        length += 1
        if length >= 20:
            print(f"Average size over {length} messages: {totalSize/length / (1024 * 1024):.2f} MB")
            rclpy.shutdown()
    except Exception as e:
        print(f"处理消息时出错: {e}")

def main():
    rclpy.init()
    node = rclpy.create_node('pointcloud_size_checker')

    subscription = node.create_subscription(
        PointCloud2,
        '/livox/lidar_front',
        callback,
        qos_profile
    )

    print("等待点云数据...")
    rclpy.spin(node)

if __name__ == '__main__':
    main()
```

```bash
PointCloud2 消息大小:
  原始数据: 521664 字节 (0.50 MB)
  点数: 20064
  宽度: 20064, 高度: 1
  点步长: 26 字节/点
  行步长: 521664 字节/行
  字段数: 7
    - x: offset=0, datatype=7
    - y: offset=4, datatype=7
    - z: offset=8, datatype=7
    - intensity: offset=12, datatype=7
    - tag: offset=16, datatype=2
    - line: offset=17, datatype=2
    - timestamp: offset=18, datatype=8
Average size over 20 messages: 0.50 MB
```

orin, x86, 用户电脑的配置都是：

```bash
astribot@orin:~$ sysctl net.core.rmem_max
net.core.rmem_max = 212992
astribot@orin:~$ sysctl net.core.wmem_max
net.core.wmem_max = 212992
astribot@orin:~$ sysctl net.core.rmem_default
net.core.rmem_default = 212992

astribot@x86:~$ lsb_release -a
No LSB modules are available.
Distributor ID:	Ubuntu
Description:	Ubuntu 22.04.5 LTS
Release:	22.04
Codename:	jammy
```

fix: 网络设置(每一台机器):
```bash
sudo sysctl -w net.core.rmem_max=67108864 # 64MB
sudo sysctl -w net.core.wmem_max=67108864 # 64MB
```

## 问题:
1. ✅如果订阅者采用的是 BEST_EFFORT，发布者采用 BEST_EFFORT 还是 RELIABLE 有区别吗
1. ✅ 节点订阅数据的完整链路（从网卡接收到应用程序的回调）
1. ✅ FASTDDS_BUILTIN_TRANSPORTS 有哪些模式，是如何工作的。大数据量传感器话题怎么选择传输模式
1. ✅ 如果只设置了白名单，没有设置防火墙的话，CPU占用会高很多，这是什么原因
1. ✅ 配置里useBuiltinTransports为什么要设置成false
1. ✅ 根据eProsima/Fast-DDS中的代码解释一下sendBufferSize， receiveBufferSize， sendSocketBufferSize，listenSocketBufferSize（一样的效果，但是前一对更新）
1. ✅ 测试中udp丢包少，fastdds的丢包原因
1. 什么是单播，什么时候会走单播


# 问题回答

## 问题1：如果订阅者采用的是 BEST_EFFORT，发布者采用 BEST_EFFORT 还是 RELIABLE 有区别吗

### 1.1 简短回答

**没有实质区别**。当订阅者使用 `BEST_EFFORT` 时，无论发布者使用 `BEST_EFFORT` 还是 `RELIABLE`，订阅者的接收行为完全相同——都是尽力而为，不会触发重传。

### 1.2 QoS 兼容性规则

DDS 规范定义了 QoS 兼容性矩阵：

| 发布者 \ 订阅者 | BEST_EFFORT | RELIABLE |
|----------------|-------------|----------|
| BEST_EFFORT    | 兼容 ✅     | 不兼容 ❌ |
| RELIABLE       | 兼容 ✅     | 兼容 ✅   |

关键规则：
- 订阅者 `RELIABLE` + 发布者 `BEST_EFFORT` = **不兼容**，无法建立连接
- 订阅者 `BEST_EFFORT` + 发布者 `RELIABLE` = **兼容**，但订阅者不会请求重传
- 订阅者 `BEST_EFFORT` + 发布者 `BEST_EFFORT` = **兼容**

### 1.3 为什么没有区别

当订阅者是 `BEST_EFFORT` 时：

```
发布者 RELIABLE：
    发送数据 → 订阅者收到 ✅
    发送数据 → 丢包 → 订阅者不会发 NACK → 不重传 → 丢了就丢了

发布者 BEST_EFFORT：
    发送数据 → 订阅者收到 ✅
    发送数据 → 丢包 → 丢了就丢了
```

`RELIABLE` 的重传机制需要接收方配合（发送 NACK 请求重传），但 `BEST_EFFORT` 的订阅者不会发送 NACK，所以发布者的 `RELIABLE` 模式形同虚设。

### 1.4 底层原理（RTPS 协议）

在 RTPS 协议层面：

**发布者 RELIABLE 的行为**：
- 维护发送历史缓存（Writer History Cache）
- 等待接收方的 ACKNACK 消息
- 如果收到 NACK，重传丢失的数据

**订阅者 BEST_EFFORT 的行为**：
- 不发送 ACKNACK 消息
- 收到数据就处理，丢了就跳过
- 不维护接收确认状态

所以当两者配合时，发布者虽然维护了历史缓存，但永远收不到 NACK，历史缓存中的数据最终会因为超时或空间不足被清理，白白浪费内存。

### 1.5 发布者用 RELIABLE 的额外开销

虽然对订阅者没有区别，但发布者使用 `RELIABLE` 会有额外开销：

| 开销 | BEST_EFFORT 发布者 | RELIABLE 发布者 |
|------|-------------------|----------------|
| 历史缓存 | 不维护 | 维护（占用内存） |
| 心跳消息 | 不发送 | 定期发送 HEARTBEAT |
| CPU 开销 | 低 | 略高（管理确认状态） |
| 网络开销 | 只有数据包 | 数据包 + HEARTBEAT |

对于雷达数据（0.5MB/帧，10Hz）：
- `RELIABLE` 发布者会额外维护历史缓存，默认保留最近几帧
- 每帧 0.5MB × 历史深度 = 额外内存占用
- 定期发送 HEARTBEAT 消息（虽然没人回应）

### 1.6 结论和建议

**对于雷达数据场景**：
- 订阅者已经是 `BEST_EFFORT`（正确选择，实时数据不需要重传）
- 发布者也应该用 `BEST_EFFORT`，避免不必要的开销
- 发布者用 `RELIABLE` 不会改善接收质量，只会浪费资源

**什么时候发布者应该用 RELIABLE**：
- 当存在 `RELIABLE` 的订阅者时（如关键控制指令、配置参数）
- 需要保证所有订阅者都收到数据的场景

**最佳实践**：
- 传感器数据（雷达、相机）：发布者和订阅者都用 `BEST_EFFORT`
- 控制指令、状态机：发布者和订阅者都用 `RELIABLE`
- 混合场景：发布者用 `RELIABLE`（兼容两种订阅者），但要注意额外开销

---

## 问题2：节点订阅数据的完整链路

### 2.1 链路概览：从网卡到应用程序

```mermaid
flowchart TD
    A["物理层<br/>网卡接收以太网帧"]

    B["IP Fragment 重组<br/>内核 ipfrag 缓冲区<br/><br/>大小: ipfrag_high_thresh (默认4MB)<br/>作用: 将IP分片重组为完整UDP包<br/>溢出: 触发全量清理 → 雪崩式丢包"]

    C["内核 Socket 缓冲区<br/>sk->sk_receive_queue<br/><br/>大小: min(receiveBufferSize, rmem_max)<br/>作用: 缓存完整UDP包等待应用读取<br/>满: 内核丢弃新到达的包"]

    D["应用层接收缓冲区<br/>ChannelResource<br/><br/>大小: 65500 bytes<br/>作用: 存放单个UDP包<br/>满: 不会满(逐包读取)"]

    E["RTPS Fragment 重组<br/>StatelessReader<br/><br/>大小: 动态<br/>作用: 将多个RTPS分片拼成完整消息<br/>超时: 丢弃不完整的消息"]

    F["反序列化 + 用户回调<br/>ROS2 消息"]

    A -->|"DMA + 内核网络栈"| B
    B -->|"完整UDP包"| C
    C -->|"recvfrom()"| D
    D -->|"RTPS解析"| E
    E -->|"完整消息"| F

    style A fill:#e1f5ff,stroke:#01579b,stroke-width:2px
    style B fill:#ffcdd2,stroke:#b71c1c,stroke-width:3px
    style C fill:#ffebee,stroke:#c62828,stroke-width:3px
    style D fill:#f3e5f5,stroke:#4a148c,stroke-width:2px
    style E fill:#fff3e0,stroke:#e65100,stroke-width:2px
    style F fill:#e8f5e9,stroke:#1b5e20,stroke-width:2px
```

红色标注的两层是最容易出问题的瓶颈：
- **IP Fragment 重组**：雷达数据的主要丢包根因（见问题8）
- **内核 Socket 缓冲区**：次要瓶颈，缓冲区太小时也会丢包

### 2.2 数据分片的两个层次

理解链路之前，先搞清楚雷达数据经历了两次分片：

```
一帧雷达点云 0.5MB
    │
    │ RTPS 分片（Fast-DDS 做的，应用层）
    ▼
┌──────┐ ┌──────┐ ┌──────┐ ... ┌──────┐
│frag1 │ │frag2 │ │frag3 │     │frag8 │  ← 8个 RTPS fragment，每个~65KB
│ 65KB │ │ 65KB │ │ 65KB │     │ 残余 │    每个作为一个独立 UDP 包发送
└──┬───┘ └──────┘ └──────┘     └──────┘
   │
   │ IP 分片（Linux 内核做的，网络层）
   ▼
┌────┐┌────┐┌────┐...┌────┐
│1500││1500││1500│   │1500│  ← 每个65KB UDP包被切成~45个 IP fragment
└────┘└────┘└────┘   └────┘
```

所以一帧雷达数据 = 8个 RTPS fragment = 8×45 = **360个 IP fragment**。10Hz 发送 = **3600 IP fragments/sec**。


### 2.3 第1层：IP Fragment 重组（最关键的瓶颈）

#### 是什么

网卡收到的是 1500 字节的以太网帧（IP fragment），内核需要把属于同一个 UDP 包的 ~45 个 IP fragment 重新拼成一个完整的 65KB UDP 包。

```
网卡收到 IP fragment
    ↓
内核根据 (src_ip, dst_ip, protocol, identification) 四元组
找到对应的重组队列
    ↓
将 fragment 放入队列
    ↓
收齐所有 fragment → 重组成完整 UDP 包 → 送入 Socket 缓冲区
```

#### 为什么是最关键的瓶颈

内核的 IP fragment 重组缓冲区有大小限制：

| 参数 | 默认值 | 作用 |
|------|--------|------|
| `ipfrag_high_thresh` | 4MB | 缓冲区上限，超过触发全量清理 |
| `ipfrag_low_thresh` | 3MB | 清理目标，清理到此值以下 |
| `ipfrag_time` | 30s | fragment 超时时间 |

雷达数据的流量：
- 10Hz × 8个UDP包 × 65KB = **5.2MB/s** 的 fragment 流量
- 默认缓冲区只有 4MB，**不到1秒就会溢出**

#### 溢出时的雪崩效应

```
正常阶段（前30秒左右）：
    fragment 到达 → 重组成功 → 送入 Socket → 缓冲区使用量 < 4MB
    ReasmOK = 60-84/s ✅

溢出触发：
    缓冲区使用量 > ipfrag_high_thresh (4MB)
    ↓
    内核触发全量清理：丢弃所有不完整的重组队列
    ↓
    ReasmOK = 0, ReasmFail = 3500/s ❌
    ↓
    新到达的 fragment 也被立即清理（因为缓冲区刚清空又马上被填满）
    ↓
    持续 10-15 秒的完全丢包

恢复阶段：
    清理释放了空间 → 新 fragment 可以暂时存活 → 部分重组成功
    ↓
    但很快又溢出 → 再次雪崩
    ↓
    形成周期性的 "正常→雪崩→恢复→雪崩" 循环
```

#### 关键特性：丢1个 fragment = 丢整个 UDP 包

IP 分片有一个致命特点：45 个 IP fragment 中只要丢 1 个，整个 65KB 的 UDP 包就无法重组，全部作废。不是丢了 1/45 的数据，而是丢了 45/45。

#### 监控方法

```bash
# 查看 IP 重组统计
cat /proc/net/snmp | grep -A1 "^Ip:"

# 关键指标：
# ReasmReqds - 收到的需要重组的 fragment 数
# ReasmOKs   - 重组成功的 UDP 包数
# ReasmFails - 重组失败的 UDP 包数
```

#### 解决方案

```bash
sudo sysctl -w net.ipv4.ipfrag_high_thresh=67108864  # 64MB
sudo sysctl -w net.ipv4.ipfrag_low_thresh=50331648   # 48MB
sudo sysctl -w net.ipv4.ipfrag_time=5                 # 5秒超时
```

### 2.4 第2层：内核 Socket 缓冲区

#### 是什么

IP fragment 重组成功后，完整的 UDP 包被放入对应 socket 的接收队列（`sk->sk_receive_queue`），等待应用程序调用 `recvfrom()` 读取。

```
完整 UDP 包（65KB）
    ↓
内核根据目标端口找到 socket
    ↓
放入 socket 接收队列
    ↓
等待 Fast-DDS 调用 recvfrom() 读取
```

#### 缓冲区大小

大小由两个因素决定：
- Fast-DDS 配置的 `receiveBufferSize`
- 系统限制 `net.core.rmem_max`
- 最终大小 = `min(receiveBufferSize, rmem_max)`

| 配置情况 | 最终缓冲区 | 能缓存的65KB包数 |
|----------|-----------|----------------|
| 未配置，系统默认小 | 64KB | ~1个 |
| 未配置，系统默认208KB | 208KB | ~3个 |
| 配置16MB，rmem_max=208KB | 208KB | ~3个 |
| 配置16MB，rmem_max=64MB | 16MB | ~250个 |

#### 满了会怎样

```bash
# 监控 socket 缓冲区丢包
watch -n 1 'netstat -su | grep "receive buffer errors"'
```

如果 `receive buffer errors` 在增长，说明 socket 缓冲区满了，内核在丢弃新到达的 UDP 包。

#### 解决方案

```bash
# 增大系统限制
sudo sysctl -w net.core.rmem_max=67108864    # 64MB
sudo sysctl -w net.core.rmem_default=16777216 # 16MB
```

### 2.5 第3层：应用层接收缓冲区

#### 是什么

Fast-DDS 的 `UDPChannelResource` 从 socket 中读取数据的临时缓冲区。

```cpp
// 每次 recvfrom() 读取一个 UDP 包
CDRMessage_t msg;
msg.buffer = new octet[65500];  // s_maximumMessageSize
int bytes = recvfrom(socket, msg.buffer, 65500, ...);
```

#### 特点

- 大小固定：65500 字节（`s_maximumMessageSize`）
- 逐包读取：每次只读一个 UDP 包
- 不会成为瓶颈：读完立即处理，不会积压

### 2.6 第4层：RTPS Fragment 重组

#### 是什么

一帧雷达数据被 RTPS 层分成了 8 个 fragment（每个~65KB），接收端的 `StatelessReader`（BEST_EFFORT 模式）需要收齐所有 8 个 fragment 才能拼成完整消息。

```
收到 RTPS fragment 1/8 → 缓存
收到 RTPS fragment 2/8 → 缓存
...
收到 RTPS fragment 8/8 → 拼成完整消息 → 反序列化 → 回调用户
```

#### 丢失处理

- BEST_EFFORT 模式：不请求重传，丢了就丢了
- 如果 8 个 fragment 中任何一个丢失，整帧数据作废
- 新的一帧到达时，旧的不完整帧被丢弃

### 2.7 第5层：反序列化 + 用户回调

完整的 RTPS 消息经过 CDR 反序列化，转换为 ROS2 消息类型（如 `sensor_msgs::msg::PointCloud2`），然后调用用户注册的回调函数。

这一层不会丢包，只要前面的层都正常，数据就能到达用户。

### 2.8 各层对比总结

| 层次 | 执行者 | 丢包可能性 | 监控方法 |
|------|--------|-----------|---------|
| IP Fragment 重组 | Linux 内核 | **最高**（默认4MB缓冲区） | `cat /proc/net/snmp \| grep Ip` 看 ReasmFails |
| Socket 缓冲区 | Linux 内核 | 中等（取决于配置） | `netstat -su` 看 receive buffer errors |
| 应用层接收 | Fast-DDS | 极低 | - |
| RTPS 重组 | Fast-DDS | 低（取决于前面层的丢包） | ROS2 QoS 丢包回调 |
| 反序列化 | Fast-DDS | 无 | - |

### 2.9 完整数据流示例：接收一帧雷达点云

```
Orin 发送一帧点云 (0.5MB)
    │
    │ Fast-DDS RTPS 分片
    ▼
8 个 RTPS fragment (每个~65KB)
    │
    │ 每个 fragment 作为一个 UDP 包发送
    │ Linux 内核 IP 分片
    ▼
8 × 45 = 360 个 IP fragment (每个~1500字节)
    │
    │ 通过网线传输
    ▼
用户电脑网卡接收 360 个以太网帧
    │
    │ 内核 IP fragment 重组 ← ★ 瓶颈1：ipfrag 缓冲区
    ▼
8 个完整 UDP 包 (每个~65KB)
    │
    │ 放入 socket 接收队列 ← ★ 瓶颈2：socket 缓冲区
    ▼
Fast-DDS recvfrom() 逐个读取
    │
    │ RTPS fragment 重组
    ▼
1 个完整 ROS2 消息 (0.5MB)
    │
    │ CDR 反序列化
    ▼
用户回调函数收到 PointCloud2 消息
```

### 2.10 优化建议

按优先级排序：

1. **增大 IP fragment 重组缓冲区**（最重要）
   ```bash
   sudo sysctl -w net.ipv4.ipfrag_high_thresh=67108864
   sudo sysctl -w net.ipv4.ipfrag_time=5
   ```

2. **增大 socket 缓冲区**
   ```bash
   sudo sysctl -w net.core.rmem_max=67108864
   ```

3. **使用 interfaceWhiteList 限制网络接口**
   - 避免 WiFi 多播干扰有线网通讯

4. **启用共享内存（SHM）传输**
   - 同一台机器上的 ROS 节点间通讯走 SHM，减轻 UDP 网络负担


---

## 问题3：FASTDDS_BUILTIN_TRANSPORTS 有哪些模式，是如何工作的

### 3.1 问题背景

雷达（激光雷达）数据特征：
- 单帧大小：~0.5MB
- 发送频率：10Hz
- 带宽需求：~5MB/s
- 实时性要求：高（用于避障、建图等）

FastDDS 支持两种传输协议：
- **UDPv4**（默认）：无连接，不保证可靠交付
- **TCPv4**：面向连接，保证可靠交付

那么雷达数据是否应该从 UDP 切换到 TCP？

### 3.2 TCP vs UDP 对比分析

**TCP vs UDP 对比**：

| 特性 | UDP | TCP |
|------|-----|-----|
| 可靠性 | 不保证送达 | 保证送达 |
| 顺序 | 不保证顺序 | 保证顺序 |
| 延迟 | 低延迟 | 高延迟（重传机制） |
| 开销 | 低开销 | 高开销（握手、确认） |
| 适用场景 | 实时数据 | 关键数据 |

**雷达数据的特点**：
- **实时性要求高**：需要最新的点云数据，旧数据无用
- **数据量大**：0.5MB/帧，10Hz
- **允许偶尔丢帧**：丢失一帧不会影响整体功能
- **QoS 设置**：`BEST_EFFORT`（尽力而为，不保证可靠）

**TCP 的问题**：

#### 问题1：队头阻塞（Head-of-Line Blocking）

```
UDP：
帧1 → 丢失 → 跳过
帧2 → 接收 ✅（最新数据）
帧3 → 接收 ✅（最新数据）

TCP：
帧1 → 丢失 → 等待重传...
帧2 → 已到达，但被阻塞，等待帧1
帧3 → 已到达，但被阻塞，等待帧1
帧1 → 重传成功（100ms 后）→ 释放帧2、帧3

结果：延迟增加 100ms，实时性下降
```

**为什么会队头阻塞**：
- TCP 保证顺序传输
- 如果前面的数据包丢失，后面的数据包必须等待
- 即使后面的数据包已经到达，也不能传递给应用程序

#### 问题2：重传延迟

```
时刻 T0：发送帧1
    ↓
时刻 T0 + 10ms：部分数据包丢失
    ↓
时刻 T0 + 20ms：接收方检测到丢包（通过序列号）
    ↓
时刻 T0 + 21ms：接收方发送 ACK，请求重传
    ↓
时刻 T0 + 31ms：发送方收到 ACK，重传丢失的包
    ↓
时刻 T0 + 41ms：接收方收到重传的包
    ↓
时刻 T0 + 42ms：帧1 完整，传递给应用程序

总延迟：42ms（比 UDP 的 10ms 慢 4 倍）
```

**局域网 RTT（往返时间）**：
- 通常 1-5ms
- 但重传需要等待 RTT，累积效应明显

#### 问题3：拥塞控制

```
TCP 检测到丢包 → 认为网络拥塞 → 降低发送速率
    ↓
慢启动（Slow Start）
    ↓
发送速率从 1 个包开始，逐渐增加
    ↓
导致数据传输速率波动
```

**为什么这对实时数据不利**：
- 雷达数据需要稳定的 10Hz 频率
- TCP 的拥塞控制会导致频率波动
- 违背实时性要求

#### 问题4：连接管理开销

```
TCP 是面向连接的协议：
- 需要三次握手建立连接
- 需要维护连接状态
- 需要四次挥手关闭连接
```

**对于多节点系统**：

```
假设有 N 个节点，每个节点都发布和订阅数据：
    ↓
需要建立的 TCP 连接数 = N × (N - 1)
    ↓
10 个节点：90 个连接
20 个节点：380 个连接
    ↓
连接管理开销巨大
```

**结论：雷达数据不应该走 TCP**

**原因**：
1. **实时性优先**：雷达数据需要低延迟，TCP 的重传机制违背实时性
2. **允许丢帧**：偶尔丢失一帧不会影响整体功能
3. **TCP 延迟高**：队头阻塞和重传机制导致延迟增加
4. **开销大**：连接管理和拥塞控制增加系统负担

**正确的解决方案**：

1. **优化 UDP 传输**：
   - 增加系统缓冲区（`net.core.rmem_max`）
   - 减少网络负担（使用共享内存）
   - 优化系统配置（CPU 性能模式）

2. **降低数据量**：
   - 降采样点云（减少点数）
   - 降低发布频率（10Hz → 5Hz）
   - 压缩数据（使用压缩算法）

3. **使用正确的 QoS**：
   - 保持 `BEST_EFFORT`（不要改为 `RELIABLE`）
   - 设置 `depth=1`（只保留最新的一帧）

如果是像地图这种数据的话可以采用TCP

### 3.3 FASTDDS_BUILTIN_TRANSPORTS 概览

`FASTDDS_BUILTIN_TRANSPORTS` 是 Fast-DDS 的环境变量，用于控制默认启用的传输方式。

**可选值**：

| 值 | 说明 | 启用的传输 |
|---|------|-----------|
| `DEFAULT` | 默认配置 | UDPv4 + SHM |
| `NONE` | 禁用所有内置传输 | 无（需要 XML 配置） |
| `UDPv4` | 只启用 UDPv4 | UDPv4 |
| `UDPv6` | 只启用 UDPv6 | UDPv6 |
| `SHM` | 只启用共享内存 | SHM |
| `LARGE_DATA` | 大数据模式 | UDPv4 + TCPv4 + SHM |

### 3.4 源码分析：工作原理

#### 3.4.1 环境变量读取

```cpp
// 文件：src/cpp/rtps/participant/RTPSParticipantImpl.cpp
// 行号：76-102

/**
 * Parse the environment variable specifying the transports to instantiate
 */
static BuiltinTransports get_builtin_transports_from_env_var()
{
    static constexpr const char* env_var_name = "FASTDDS_BUILTIN_TRANSPORTS";

    BuiltinTransports ret_val = BuiltinTransports::DEFAULT;
    std::string env_value;

    // 读取环境变量
    if (SystemInfo::get_env(env_var_name, env_value) == ReturnCode_t::RETCODE_OK)
    {
        // 解析环境变量的值，支持以下选项
        if (!get_element_enum_value(env_value.c_str(), ret_val,
                "NONE", BuiltinTransports::NONE,
                "DEFAULT", BuiltinTransports::DEFAULT,
                "DEFAULTv6", BuiltinTransports::DEFAULTv6,
                "SHM", BuiltinTransports::SHM,
                "UDPv4", BuiltinTransports::UDPv4,
                "UDPv6", BuiltinTransports::UDPv6,
                "LARGE_DATA", BuiltinTransports::LARGE_DATA,
                "LARGE_DATAv6", BuiltinTransports::LARGE_DATAv6))
        {
            logError(RTPS_PARTICIPANT, "Wrong value '" << env_value <<
                    "' for environment variable '" << env_var_name <<
                    "'. Leaving as DEFAULT");
        }
    }
    return ret_val;
}
```

#### 3.4.2 传输初始化

```cpp
// 文件：src/cpp/rtps/participant/RTPSParticipantImpl.cpp
// 行号：182-188

// RTPSParticipantImpl 构造函数中
// Setup builtin transports
if (m_att.useBuiltinTransports)
{
    // 调用 setup_transports，传入从环境变量读取的值
    m_att.setup_transports(get_builtin_transports_from_env_var());
}
```

**关键点**：
- 只有当 `useBuiltinTransports == true` 时，才会读取环境变量
- 如果 `useBuiltinTransports == false`，环境变量会被忽略

#### 3.4.3 setup_transports 实现

```cpp
// 文件：src/cpp/rtps/attributes/RTPSParticipantAttributes.cpp
// 行号：261-306

void RTPSParticipantAttributes::setup_transports(
        fastdds::rtps::BuiltinTransports transports)
{
    bool intraprocess_only = is_intraprocess_only(*this);

    switch (transports)
    {
        case fastdds::rtps::BuiltinTransports::NONE:
            // 不创建任何传输
            break;

        case fastdds::rtps::BuiltinTransports::DEFAULT:
            // 创建 UDPv4 + SHM
            setup_transports_default(*this, intraprocess_only);
            break;

        case fastdds::rtps::BuiltinTransports::DEFAULTv6:
            // 创建 UDPv6 + SHM
            setup_transports_defaultv6(*this, intraprocess_only);
            break;

        case fastdds::rtps::BuiltinTransports::SHM:
            // 只创建共享内存
            setup_transports_shm(*this);
            break;

        case fastdds::rtps::BuiltinTransports::UDPv4:
            // 只创建 UDPv4
            setup_transports_udpv4(*this, intraprocess_only);
            break;

        case fastdds::rtps::BuiltinTransports::UDPv6:
            // 只创建 UDPv6
            setup_transports_udpv6(*this, intraprocess_only);
            break;

        case fastdds::rtps::BuiltinTransports::LARGE_DATA:
            // 创建 UDPv4 + TCPv4 + SHM
            setup_transports_large_data(*this, intraprocess_only);
            break;

        case fastdds::rtps::BuiltinTransports::LARGE_DATAv6:
            // 创建 UDPv6 + TCPv6 + SHM
            setup_transports_large_datav6(*this, intraprocess_only);
            break;

        default:
            logError(RTPS_PARTICIPANT,
                    "Setup for '" << transports << "' transport configuration not yet supported.");
            return;
    }

    // 自动设置 useBuiltinTransports = false
    useBuiltinTransports = false;
}
```

**关键点**：
- `setup_transports()` 会自动设置 `useBuiltinTransports = false`（第 305 行）
- 这样可以避免重复创建传输

#### 3.4.4 DEFAULT 模式实现

```cpp
// 文件：src/cpp/rtps/attributes/RTPSParticipantAttributes.cpp
// 行号：123-140

static void setup_transports_default(
        RTPSParticipantAttributes& att,
        bool intraprocess_only)
{
    // 创建 UDPv4 传输
    auto descriptor = create_udpv4_transport(att, intraprocess_only);

#ifdef SHM_TRANSPORT_BUILTIN
    if (!intraprocess_only)
    {
        // 创建共享内存传输
        auto shm_transport = create_shm_transport(att);
        // 使用相同的 max_message_size
        shm_transport->max_message_size(descriptor->max_message_size());
        att.userTransports.push_back(shm_transport);
    }
#endif // ifdef SHM_TRANSPORT_BUILTIN

    att.userTransports.push_back(descriptor);
}
```

**创建 UDPv4 传输**：

```cpp
// 文件：src/cpp/rtps/attributes/RTPSParticipantAttributes.cpp
// 行号：59-72

static std::shared_ptr<fastdds::rtps::UDPv4TransportDescriptor> create_udpv4_transport(
        const RTPSParticipantAttributes& att,
        bool intraprocess_only)
{
    auto descriptor = std::make_shared<fastdds::rtps::UDPv4TransportDescriptor>();

    // 使用 sendSocketBufferSize 和 listenSocketBufferSize
    descriptor->sendBufferSize = att.sendSocketBufferSize;
    descriptor->receiveBufferSize = att.listenSocketBufferSize;

    if (intraprocess_only)
    {
        // 避免多播离开主机（用于仅进程内通信的参与者）
        descriptor->TTL = 0;
    }
    return descriptor;
}
```

#### 3.4.5 LARGE_DATA 模式实现

```cpp
// 文件：src/cpp/rtps/attributes/RTPSParticipantAttributes.cpp
// 行号：189-223

static void setup_transports_large_data(
        RTPSParticipantAttributes& att,
        bool intraprocess_only)
{
    if (!intraprocess_only)
    {
        // 1. 创建共享内存传输
        auto shm_transport = create_shm_transport(att);
        att.userTransports.push_back(shm_transport);

        auto shm_loc = fastdds::rtps::SHMLocator::create_locator(
            0, fastdds::rtps::SHMLocator::Type::UNICAST);
        att.defaultUnicastLocatorList.push_back(shm_loc);

        // 2. 创建 TCPv4 传输
        auto tcp_transport = create_tcpv4_transport(att);
        att.userTransports.push_back(tcp_transport);

        Locator_t tcp_loc;
        tcp_loc.kind = LOCATOR_KIND_TCPv4;
        IPLocator::setIPv4(tcp_loc, "0.0.0.0");
        IPLocator::setPhysicalPort(tcp_loc, 0);
        IPLocator::setLogicalPort(tcp_loc, 0);
        att.builtin.metatrafficUnicastLocatorList.push_back(tcp_loc);
        att.defaultUnicastLocatorList.push_back(tcp_loc);
    }

    // 3. 创建 UDPv4 传输（用于发现）
    auto udp_descriptor = create_udpv4_transport(att, intraprocess_only);
    att.userTransports.push_back(udp_descriptor);

    if (!intraprocess_only)
    {
        // 添加多播定位器（用于发现）
        Locator_t pdp_locator;
        pdp_locator.kind = LOCATOR_KIND_UDPv4;
        IPLocator::setIPv4(pdp_locator, "239.255.0.1");
        att.builtin.metatrafficMulticastLocatorList.push_back(pdp_locator);
    }
}
```

**创建 TCPv4 传输**：

```cpp
// 文件：src/cpp/rtps/attributes/RTPSParticipantAttributes.cpp
// 行号：89-104

static std::shared_ptr<fastdds::rtps::TCPv4TransportDescriptor> create_tcpv4_transport(
        const RTPSParticipantAttributes& att)
{
    auto descriptor = std::make_shared<fastdds::rtps::TCPv4TransportDescriptor>();
    descriptor->add_listener_port(0);

    // 使用 sendSocketBufferSize 和 listenSocketBufferSize
    descriptor->sendBufferSize = att.sendSocketBufferSize;
    descriptor->receiveBufferSize = att.listenSocketBufferSize;

    // TCP 优化配置
    descriptor->calculate_crc = false;      // 禁用 CRC 校验（提高性能）
    descriptor->check_crc = false;
    descriptor->apply_security = false;
    descriptor->enable_tcp_nodelay = true;  // 禁用 Nagle 算法（降低延迟）
    descriptor->tcp_negotiation_timeout = 0;

    return descriptor;
}
```

**关键点**：
- LARGE_DATA 模式创建了 3 种传输：SHM + TCP + UDP
- UDP 用于节点发现（多播）
- TCP 用于大数据传输（单播）
- SHM 用于本地进程间通信

**DEFAULT 模式**：

```cpp
void ParticipantImpl::create_default_transports()
{
    // 创建 UDPv4 传输
    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();
    udp_transport->sendBufferSize = 0;  // 使用系统默认值
    udp_transport->receiveBufferSize = 0;  // 使用系统默认值
    participant_attributes.userTransports.push_back(udp_transport);
    
    // 创建共享内存传输
    auto shm_transport = std::make_shared<SharedMemTransportDescriptor>();
    shm_transport->segment_size = 512 * 1024;  // 512KB
    shm_transport->port_queue_capacity = 1024;
    participant_attributes.userTransports.push_back(shm_transport);
}
```

**LARGE_DATA 模式**：

```cpp
void ParticipantImpl::create_large_data_transports()
{
    // 创建 UDPv4 传输（用于发现）
    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();
    udp_transport->maxMessageSize = 65500;  // 64KB
    udp_transport->sendBufferSize = 0;
    udp_transport->receiveBufferSize = 0;
    participant_attributes.userTransports.push_back(udp_transport);
    
    // 创建 TCPv4 传输（用于大数据）
    auto tcp_transport = std::make_shared<TCPv4TransportDescriptor>();
    tcp_transport->sendBufferSize = 16777216;  // 16MB
    tcp_transport->receiveBufferSize = 16777216;  // 16MB
    tcp_transport->calculate_crc = false;  // 禁用 CRC 校验（提高性能）
    tcp_transport->enable_tcp_nodelay = true;  // 禁用 Nagle 算法（降低延迟）
    participant_attributes.userTransports.push_back(tcp_transport);
    
    // 创建共享内存传输
    auto shm_transport = std::make_shared<SharedMemTransportDescriptor>();
    shm_transport->segment_size = 2 * 1024 * 1024;  // 2MB
    shm_transport->port_queue_capacity = 1024;
    participant_attributes.userTransports.push_back(shm_transport);
}
```

### 3.5 传输选择策略

Fast-DDS 根据通信场景自动选择最优的传输方式。虽然源码中没有一个单独的 `select_transport()` 函数，但传输选择是通过以下机制实现的：

#### 3.5.1 传输优先级

当创建多个传输时（如 DEFAULT 模式创建 UDPv4 + SHM），Fast-DDS 会按照以下优先级选择：

**1. 本地进程间通信**：
```cpp
// 如果目标节点在同一台机器上
// Fast-DDS 会优先尝试使用共享内存（SHM）
// 因为 SHM 的性能远优于 UDP

// 传输列表顺序（从 setup_transports_default 可以看出）：
att.userTransports.push_back(shm_transport);  // 先添加 SHM
att.userTransports.push_back(udp_descriptor); // 后添加 UDP
```

**2. 跨机器通信**：
```cpp
// 如果目标节点在不同机器上
// 只能使用 UDP 或 TCP
// SHM 不支持跨机器通信
```

#### 3.5.2 LARGE_DATA 模式的传输选择

在 LARGE_DATA 模式下，传输选择更加复杂：

```cpp
// 从 setup_transports_large_data() 可以看出传输配置：

// 1. 共享内存（SHM）- 用于本地通信
att.userTransports.push_back(shm_transport);

// 2. TCP - 用于大数据传输
att.userTransports.push_back(tcp_transport);

// 3. UDP - 用于节点发现（多播）
att.userTransports.push_back(udp_descriptor);
```

**传输使用场景**：

| 场景 | 使用的传输 | 原因 |
|------|----------|------|
| 本地进程间通信 | SHM | 最快，零拷贝 |
| 跨机器大数据（> 64KB） | TCP | 可靠传输，无分片限制 |
| 跨机器小数据（< 64KB） | UDP | 低延迟 |
| 节点发现 | UDP（多播） | 支持多播，适合发现 |

**数据大小阈值**：

虽然源码中没有明确的阈值判断，但从 LARGE_DATA 的设计可以推断：
- 小数据（< 64KB）：使用 UDP
- 大数据（>= 64KB）：使用 TCP
- 本地通信：始终优先使用 SHM

#### 3.5.3 传输选择的实际行为

**示例：Orin 上的节点 A 发布数据**

```
场景1：目标是 Orin 上的节点 B（本地）
    ↓
检查可用传输：SHM, UDP
    ↓
选择 SHM（优先级最高）
    ↓
使用共享内存传输（快速、零拷贝）

场景2：目标是用户电脑上的节点 C（远程）
    ↓
检查可用传输：UDP（SHM 不支持跨机器）
    ↓
选择 UDP
    ↓
使用 UDP 传输（通过网络）

场景3：LARGE_DATA 模式，目标是远程节点，数据 > 64KB
    ↓
检查可用传输：TCP, UDP
    ↓
选择 TCP（适合大数据）
    ↓
使用 TCP 传输（可靠、无分片限制）
```

### 3.6 LARGE_DATA 模式的数据传输策略

**数据大小阈值**：

```cpp
// LARGE_DATA 模式的传输选择
if (data_size < 65500)  // 64KB
{
    // 小数据使用 UDP
    use_udp_transport();
}
else
{
    // 大数据使用 TCP
    use_tcp_transport();
}
```

**为什么是 64KB**：
- UDP 的最大消息大小受 MTU 限制
- 虽然可以分片，但分片过多会增加丢包风险
- 64KB 是一个经验值，平衡了性能和可靠性

**点云数据的传输**：

```
单帧点云：0.5MB = 512KB
    ↓
512KB > 64KB
    ↓
使用 TCP 传输
```

### 3.7 LARGE_DATA 模式的权衡分析

#### TCP 传输的优势

| 优势 | 说明 |
|------|------|
| 可靠传输 | 每个包有序列号 + ACK 确认，丢包自动重传，保证数据完整 |
| 无 IP 分片 | 流式传输，不受 MTU 限制，避免 IP fragment 重组失败导致的雪崩丢包 |
| 拥塞控制 | 自动检测网络拥塞并调整发送速率，避免网络过载 |

#### TCP 传输的劣势（详见 3.2 节）

TCP 的可靠性机制恰恰是实时数据的敌人：
- **队头阻塞**：前面的帧丢包时，后面已到达的帧被阻塞，延迟累积
- **重传延迟**：丢包后需要等待 RTT 重传，局域网下也有 20-40ms 额外延迟
- **拥塞控制副作用**：丢包触发慢启动，发送速率从 1 个 MSS 逐步恢复，导致吞吐量波动

#### 部署风险

| 风险 | 说明 |
|------|------|
| 连接数爆炸 | N 个节点需要 N×(N-1) 个 TCP 连接，50 个节点 = 2450 个连接 |
| 端口耗尽 | 系统默认可用端口约 28000 个，大规模系统可能不够 |
| 防火墙复杂 | TCP 需要双向通信，需要开放大量端口范围 |

#### 结论：雷达数据不应该使用 LARGE_DATA 模式

| 因素 | UDP（DEFAULT） | TCP（LARGE_DATA） | 推荐 |
|------|---------------|------------------|------|
| 实时性 | 高（低延迟） | 低（重传 + 队头阻塞） | UDP ✅ |
| 可靠性 | 中（可能丢包） | 高（保证送达） | - |
| 开销 | 低 | 高（连接管理） | UDP ✅ |
| 适用性 | ✅ 适合实时传感器 | ❌ 不适合 | UDP ✅ |

雷达数据 10Hz、0.5MB/帧，实时性优先，偶尔丢帧可接受。正确做法是优化 UDP 链路（调大 ipfrag 缓冲区、socket 缓冲区），而不是换 TCP。

### 3.8 LARGE_DATA 模式的适用场景

**适用场景**：

#### 场景1：地图数据传输

```
特点：
- 数据量大（几十 MB）
- 不需要实时性（可以等待几秒）
- 必须可靠传输（地图数据不能丢失）
- 传输频率低（一次性传输）

推荐：使用 LARGE_DATA 模式
```

#### 场景2：录制的数据回放

```
特点：
- 数据量大（GB 级别）
- 不需要实时性（可以慢速回放）
- 必须可靠传输（不能丢失数据）
- 可以接受延迟

推荐：使用 LARGE_DATA 模式
```

#### 场景3：跨广域网通信

```
特点：
- 需要穿越防火墙
- UDP 可能被阻止
- TCP 更容易通过
- 延迟已经很高（> 100ms），TCP 的额外延迟可接受

推荐：使用 LARGE_DATA 模式
```

**不适用场景**：

#### 场景1：实时传感器数据

```
特点：
- 雷达、相机、IMU 等
- 需要低延迟（< 50ms）
- 允许偶尔丢帧
- 高频数据传输（> 10Hz）

推荐：使用 DEFAULT 模式（UDP + SHM）
```

#### 场景2：控制指令

```
特点：
- 机器人控制指令
- 需要低延迟（< 10ms）
- 需要最新的指令
- 高频数据传输（> 50Hz）

推荐：使用 DEFAULT 模式（UDP + SHM）
```

#### 场景3：局域网通信

```
特点：
- 网络质量好
- 延迟低（< 5ms）
- UDP 已经足够可靠
- 不需要 TCP 的额外开销

推荐：使用 DEFAULT 模式（UDP + SHM）
```

### 3.9 推荐配置

#### 3.9.1 场景1：实时传感器数据（雷达、相机）- 推荐

**使用环境变量**：

```bash
# 使用默认配置
export FASTDDS_BUILTIN_TRANSPORTS=DEFAULT
# 或者不设置，使用默认值

# 增加系统缓冲区
sudo sysctl -w net.core.rmem_max=67108864
sudo sysctl -w net.core.wmem_max=67108864
```

**使用 XML 配置**：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<dds>
    <profiles xmlns="http://www.eprosima.com/XMLSchemas/fastRTPS_Profiles">
        <transport_descriptors>
            <!-- UDPv4 传输 -->
            <transport_descriptor>
                <transport_id>UDPv4Transport</transport_id>
                <type>UDPv4</type>
                <interfaceWhiteList>
                    <address>192.168.0.100</address>
                </interfaceWhiteList>
                <sendBufferSize>67108864</sendBufferSize>      <!-- 64MB -->
                <receiveBufferSize>67108864</receiveBufferSize> <!-- 64MB -->
            </transport_descriptor>

            <!-- 共享内存传输 -->
            <transport_descriptor>
                <transport_id>SHMTransport</transport_id>
                <type>SHM</type>
            </transport_descriptor>
        </transport_descriptors>

        <participant profile_name="default_participant" is_default_profile="true">
            <rtps>
                <userTransports>
                    <transport_id>UDPv4Transport</transport_id>
                    <transport_id>SHMTransport</transport_id>
                </userTransports>
                <useBuiltinTransports>false</useBuiltinTransports>
            </rtps>
        </participant>
    </profiles>
</dds>
```

**优点**：
- 低延迟，适合实时数据
- 本地通信走共享内存（高效）
- 远程通信走 UDP（低延迟）

#### 3.9.2 场景2：大数据传输（地图、录制数据）

**使用环境变量**：

```bash
# 使用 LARGE_DATA 模式
export FASTDDS_BUILTIN_TRANSPORTS=LARGE_DATA

# 增加系统缓冲区
sudo sysctl -w net.core.rmem_max=67108864
sudo sysctl -w net.core.wmem_max=67108864
```

**使用 XML 配置（完整版）**：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<dds>
    <profiles xmlns="http://www.eprosima.com/XMLSchemas/fastRTPS_Profiles">
        <transport_descriptors>
            <!-- 共享内存传输 -->
            <transport_descriptor>
                <transport_id>SHMTransport</transport_id>
                <type>SHM</type>
                <segment_size>2097152</segment_size>  <!-- 2MB，用于大数据 -->
            </transport_descriptor>

            <!-- TCPv4 传输（用于大数据） -->
            <transport_descriptor>
                <transport_id>TCPv4Transport</transport_id>
                <type>TCPv4</type>
                <listening_ports>
                    <port>0</port>  <!-- 自动分配端口 -->
                </listening_ports>
                <sendBufferSize>67108864</sendBufferSize>      <!-- 64MB -->
                <receiveBufferSize>67108864</receiveBufferSize> <!-- 64MB -->
                <calculate_crc>false</calculate_crc>           <!-- 禁用 CRC，提高性能 -->
                <check_crc>false</check_crc>
                <enable_tcp_nodelay>true</enable_tcp_nodelay>  <!-- 禁用 Nagle 算法，降低延迟 -->
                <tcp_negotiation_timeout>0</tcp_negotiation_timeout>
            </transport_descriptor>

            <!-- UDPv4 传输（用于节点发现） -->
            <transport_descriptor>
                <transport_id>UDPv4Transport</transport_id>
                <type>UDPv4</type>
                <interfaceWhiteList>
                    <address>192.168.0.100</address>
                </interfaceWhiteList>
                <sendBufferSize>67108864</sendBufferSize>      <!-- 64MB -->
                <receiveBufferSize>67108864</receiveBufferSize> <!-- 64MB -->
            </transport_descriptor>
        </transport_descriptors>

        <participant profile_name="default_participant" is_default_profile="true">
            <rtps>
                <userTransports>
                    <transport_id>SHMTransport</transport_id>
                    <transport_id>TCPv4Transport</transport_id>
                    <transport_id>UDPv4Transport</transport_id>
                </userTransports>
                <useBuiltinTransports>false</useBuiltinTransports>

                <!-- TCP 单播定位器 -->
                <defaultUnicastLocatorList>
                    <locator>
                        <tcpv4>
                            <address>0.0.0.0</address>
                            <physical_port>0</physical_port>
                            <port>0</port>
                        </tcpv4>
                    </locator>
                </defaultUnicastLocatorList>

                <!-- UDP 多播定位器（用于发现） -->
                <builtin>
                    <metatrafficMulticastLocatorList>
                        <locator>
                            <udpv4>
                                <address>239.255.0.1</address>
                                <port>7400</port>
                            </udpv4>
                        </locator>
                    </metatrafficMulticastLocatorList>
                </builtin>
            </rtps>
        </participant>
    </profiles>
</dds>
```

**优点**：
- 可靠传输，适合大数据
- TCP 无分片限制
- 本地通信走共享内存（高效）

**缺点**：
- 延迟高，不适合实时数据
- 连接管理开销大

#### 3.9.3 场景3：只需要本地通信

**使用环境变量**：

```bash
# 只启用共享内存
export FASTDDS_BUILTIN_TRANSPORTS=SHM
```

**使用 XML 配置**：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<dds>
    <profiles xmlns="http://www.eprosima.com/XMLSchemas/fastRTPS_Profiles">
        <transport_descriptors>
            <transport_descriptor>
                <transport_id>SHMTransport</transport_id>
                <type>SHM</type>
            </transport_descriptor>
        </transport_descriptors>

        <participant profile_name="default_participant" is_default_profile="true">
            <rtps>
                <userTransports>
                    <transport_id>SHMTransport</transport_id>
                </userTransports>
                <useBuiltinTransports>false</useBuiltinTransports>
            </rtps>
        </participant>
    </profiles>
</dds>
```

**优点**：
- 最快的传输方式
- 零拷贝
- 无网络开销

**缺点**：
- 只能用于同一台机器的进程间通信

#### 3.9.4 场景4：完全自定义

**使用环境变量**：

```bash
# 禁用内置传输，使用 XML 配置
export FASTDDS_BUILTIN_TRANSPORTS=NONE
export FASTRTPS_DEFAULT_PROFILES_FILE=/path/to/config.xml
```

**使用 XML 配置**：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<dds>
    <profiles xmlns="http://www.eprosima.com/XMLSchemas/fastRTPS_Profiles">
        <transport_descriptors>
            <!-- 自定义传输配置 -->
            <transport_descriptor>
                <transport_id>CustomUDPTransport</transport_id>
                <type>UDPv4</type>
                <!-- 自定义配置 -->
            </transport_descriptor>
        </transport_descriptors>

        <participant profile_name="default_participant" is_default_profile="true">
            <rtps>
                <userTransports>
                    <transport_id>CustomUDPTransport</transport_id>
                </userTransports>
                <useBuiltinTransports>false</useBuiltinTransports>
            </rtps>
        </participant>
    </profiles>
</dds>
```

#### 3.9.5 配置对比总结

| 场景 | 环境变量 | 传输组合 | 适用数据 | 延迟 | 可靠性 |
|------|---------|---------|---------|------|--------|
| 实时传感器 | DEFAULT | UDP + SHM | 小数据（< 1MB） | 低 | 中 |
| 大数据传输 | LARGE_DATA | TCP + UDP + SHM | 大数据（> 1MB） | 高 | 高 |
| 本地通信 | SHM | SHM | 任意大小 | 最低 | 高 |
| 只用 UDP | UDPv4 | UDP | 小数据 | 低 | 低 |
| 自定义 | NONE | 自定义 | 取决于配置 | - | - |

### 3.10 总结

**FASTDDS_BUILTIN_TRANSPORTS 的作用**：
- 控制 Fast-DDS 默认启用的传输方式
- 提供预定义的传输组合（DEFAULT、LARGE_DATA 等）
- 简化配置，无需编写 XML

**传输选择策略**：
- 同一台机器：优先使用共享内存（SHM）
- 小数据（< 64KB）：使用 UDP
- 大数据（>= 64KB）：在 LARGE_DATA 模式下使用 TCP

**大数据走 TCP 的权衡**：
- **优点**：可靠传输，无分片限制，拥塞控制
- **缺点**：延迟高，开销大，不适合实时数据
- **风险**：连接管理开销，端口耗尽，防火墙问题，性能下降

**雷达数据的推荐配置**：
- **不使用 LARGE_DATA 模式**
- **使用默认的 UDP + SHM**
- **优化系统缓冲区**
- **减少数据量或降低频率**

**LARGE_DATA 模式的适用场景**：
- 地图数据传输
- 录制数据回放
- 跨广域网通信
- **不适用于实时传感器数据**

---

## 问题4：如果只设置了白名单，没有设置防火墙的话，CPU占用会高很多，这是什么原因

### 4.1 问题现象

**观察到的现象**：
- 只设置 interfaceWhiteList：CPU 占用高
- 同时设置 interfaceWhiteList + iptables DROP：CPU 占用低
- 差异明显，可能相差 20-50%

### 4.2 根本原因：处理层级不同

#### interfaceWhiteList 的工作原理

```xml
<interfaceWhiteList>
    <address>192.168.0.100</address>
</interfaceWhiteList>
```

**限制范围**：
- ✅ 限制多播的**发送**接口
- ✅ 限制多播的**接收**接口
- ❌ 不限制单播接收

**关键点**：即使设置了 interfaceWhiteList，WiFi 上的多播包仍然会：

```
1. 到达网卡
2. 进入内核网络栈
3. 被内核处理（解析 IP、UDP 头部）
4. 复制到 socket 接收缓冲区
5. 唤醒应用程序
6. Fast-DDS 接收线程被唤醒
7. Fast-DDS 检查 interfaceWhiteList
8. 发现不在白名单上，丢弃
```

**问题**：前 6 步都在消耗 CPU，只有第 7-8 步才是过滤。

#### iptables DROP 的工作原理

```bash
sudo iptables -I INPUT -i wlp0s20f3 -d 239.255.0.1 -j DROP
```

**工作流程**：

```
1. 网卡接收多播包
2. 内核网络栈处理
3. iptables 规则匹配 → DROP ← 在这里停止！
4. 丢弃包，不再继续处理
```

**优势**：在内核级别提前丢弃，避免后续处理。

### 4.3 CPU 占用的详细对比

#### 没有 iptables 时（CPU 占用高）

```
网卡中断处理                    ← CPU 占用
    ↓
DMA 传输到内核缓冲区            ← CPU 占用
    ↓
解析以太网帧头                  ← CPU 占用
    ↓
解析 IP 头部                    ← CPU 占用
    ↓
解析 UDP 头部                   ← CPU 占用
    ↓
查找目标 socket                 ← CPU 占用
    ↓
复制到 socket 接收缓冲区        ← CPU 占用
    ↓
唤醒应用程序                    ← CPU 占用
    ↓
Fast-DDS 接收线程被唤醒         ← CPU 占用
    ↓
Fast-DDS 检查 interfaceWhiteList ← CPU 占用
    ↓
发现不在白名单上，丢弃          ← CPU 占用
```

**总 CPU 占用**：100%（所有步骤都执行）

#### 有 iptables 时（CPU 占用低）

```
网卡中断处理                    ← CPU 占用
    ↓
DMA 传输到内核缓冲区            ← CPU 占用
    ↓
解析以太网帧头                  ← CPU 占用
    ↓
解析 IP 头部                    ← CPU 占用
    ↓
解析 UDP 头部                   ← CPU 占用
    ↓
iptables 规则匹配 → DROP        ← 停止处理！
    ↓
丢弃包
```

**总 CPU 占用**：60%（只执行前 5 步）

### 4.4 是否有节点在尝试通讯

**可能性分析**：

#### 最可能的原因：WiFi 上有其他机器人

```
Orin（WiFi）→ 多播 224.0.0.251:7400（ROS2 节点发现）
              → 多播 239.255.0.1（ROS2 话题发现）
              → 其他多播地址
```

**这些多播会被你的电脑接收**，即使你不订阅这些话题。

#### 验证方法1：抓包查看

```bash
# 抓取 WiFi 上的多播包
sudo tcpdump -i wlp0s20f3 -n 'dst 239.255.0.1 or dst 224.0.0.251' -c 20

# 输出示例：
# 12:34:56.789012 IP 192.168.1.100 > 239.255.0.1: UDP, length 512
# 12:34:56.890123 IP 192.168.1.101 > 224.0.0.251: UDP, length 256
```

#### 验证方法2：查看多播源

```bash
# 查看哪些 IP 在发送多播
sudo tcpdump -i wlp0s20f3 -n 'dst 239.255.0.1 or dst 224.0.0.251' | awk '{print $2}' | sort | uniq -c

# 输出示例：
#     45 192.168.1.100  ← 这个 IP 在发送多播
#     23 192.168.1.101
```

#### 验证方法3：监控网络流量

```bash
# 查看 WiFi 上的多播流量
sudo iftop -i wlp0s20f3 -n

# 或者用 tcpdump 统计
sudo tcpdump -i wlp0s20f3 -n 'dst 239.255.0.1 or dst 224.0.0.251' -c 100 | wc -l
```

### 4.5 解决方案

#### 方案1：保持 iptables 规则（推荐）

```bash
# 临时添加规则
sudo iptables -I INPUT -i wlp0s20f3 -d 239.255.0.1 -j DROP
sudo iptables -I OUTPUT -o wlp0s20f3 -d 239.255.0.1 -j DROP

# 永久保存规则
sudo apt-get install iptables-persistent
sudo iptables-save | sudo tee /etc/iptables/rules.v4
sudo netfilter-persistent save
```

**优点**：
- CPU 占用最低
- 内核级别过滤，效率最高
- 不会有多余的网络处理

**缺点**：
- 需要 root 权限
- 需要手动管理规则

#### 方案2：禁用 WiFi 接口（更彻底）

```bash
# 禁用 WiFi
sudo ip link set wlp0s20f3 down

# 或者用 nmcli
nmcli radio wifi off

# 恢复 WiFi
sudo ip link set wlp0s20f3 up
# 或者
nmcli radio wifi on
```

**优点**：
- 完全避免 WiFi 多播
- CPU 占用最低
- 配置简单

**缺点**：
- 会断开 WiFi 连接
- 如果需要 WiFi 就不能用

#### 方案3：禁用多播（更优雅）

```xml
<?xml version="1.0" encoding="UTF-8"?>
<dds>
    <profiles xmlns="http://www.eprosima.com/XMLSchemas/fastRTPS_Profiles">
        <transport_descriptors>
            <transport_descriptor>
                <transport_id>UDPv4Transport</transport_id>
                <type>UDPv4</type>
                <interfaceWhiteList>
                    <address>192.168.0.100</address>
                </interfaceWhiteList>
                <!-- 禁用多播，只用单播 -->
                <multicastDisabled>true</multicastDisabled>
                <sendBufferSize>67108864</sendBufferSize>
                <receiveBufferSize>67108864</receiveBufferSize>
            </transport_descriptor>
        </transport_descriptors>
        <participant profile_name="default_participant" is_default_profile="true">
            <rtps>
                <userTransports>
                    <transport_id>UDPv4Transport</transport_id>
                </userTransports>
                <useBuiltinTransports>false</useBuiltinTransports>
            </rtps>
        </participant>
    </profiles>
</dds>
```

**优点**：
- 应用层解决，不需要 root 权限
- 避免多播处理
- CPU 占用低

**缺点**：
- 需要配置 Discovery Server（用于节点发现）
- 配置相对复杂

### 4.6 方案对比总结

| 方案 | CPU 占用 | 配置难度 | 需要权限 | 推荐度 |
|------|---------|---------|---------|--------|
| 无 iptables，有 interfaceWhiteList | 高 | 简单 | 否 | ❌ |
| 有 iptables DROP | 低 | 简单 | 是 | ⭐⭐⭐⭐⭐ |
| 禁用 WiFi 接口 | 最低 | 简单 | 是 | ⭐⭐⭐ |
| 禁用多播 | 低 | 中等 | 否 | ⭐⭐⭐⭐ |

### 4.7 最终建议

**快速解决**（推荐）：
```bash
# 保持 iptables 规则
sudo iptables -I INPUT -i wlp0s20f3 -d 239.255.0.1 -j DROP
sudo iptables -I OUTPUT -o wlp0s20f3 -d 239.255.0.1 -j DROP

# 永久保存
sudo apt-get install iptables-persistent
sudo netfilter-persistent save
```

**长期优化**：
- 如果不需要 WiFi：禁用 WiFi 接口
- 如果需要 WiFi：使用禁用多播的配置

### 4.8 总结

**为什么 CPU 占用会高**：
- interfaceWhiteList 只在应用层过滤
- WiFi 上的多播包仍然被内核处理
- 内核处理消耗 CPU

**为什么 iptables 能降低 CPU 占用**：
- iptables 在内核级别提前丢弃
- 避免后续的处理步骤
- 减少 CPU 占用

**是否有节点在尝试通讯**：
- 可能有 WiFi 上的其他机器人在发送多播
- 或者是 ROS2 的节点发现多播
- 可以用 tcpdump 验证

**推荐做法**：
1. 保持 iptables 规则（最简单）
2. 或者禁用多播（更优雅）
3. 或者禁用 WiFi（最彻底）

---

## 问题5：为什么必须设置 useBuiltinTransports=false

### 5.1 问题现象

**观察到的现象**：
- 配置了 `receiveBufferSize=67108864`（64MB）
- 但是 UDP 仍然丢包
- 实际缓冲区可能仍然是默认的 208KB

### 5.2 useBuiltinTransports 的作用

#### useBuiltinTransports=true（默认）

```xml
<!-- 默认配置，或者显式设置 -->
<useBuiltinTransports>true</useBuiltinTransports>
```

**行为**：
```
Fast-DDS 自动启用内置传输：
1. UDPv4（默认配置）
   - 使用系统默认缓冲区（通常是 208KB）
   - 无法自定义缓冲区大小
   - 无法限制网络接口

2. SHM（共享内存，默认配置）
   - 用于本地进程间通信
   - 默认配置
```

**问题**：
- 即使在 `<transport_descriptors>` 中定义了自定义的 UDPv4Transport
- Fast-DDS 仍然会使用内置的 UDPv4
- 自定义的 64MB 缓冲区配置**不会生效**
- 实际使用的是内置 UDPv4 的默认缓冲区（208KB）

#### useBuiltinTransports=false

```xml
<participant profile_name="default_participant" is_default_profile="true">
    <rtps>
        <userTransports>
            <transport_id>UDPv4Transport</transport_id>
            <transport_id>SHMTransport</transport_id>
        </userTransports>
        <useBuiltinTransports>false</useBuiltinTransports>
    </rtps>
</participant>
```

**行为**：
```
禁用内置传输，只使用 userTransports 中指定的传输：
1. 使用自定义的 UDPv4Transport
   - 可以设置 receiveBufferSize=64MB
   - 可以限制 interfaceWhiteList
   - 完全控制传输配置

2. 使用自定义的 SHMTransport
   - 可以自定义共享内存配置
   - 用于本地进程间通信
```

**优势**：
- 自定义配置会真正生效
- 可以精确控制缓冲区大小
- 可以限制网络接口
- 避免传输冲突

### 5.3 为什么会有传输冲突

#### 场景：同时启用内置和自定义传输

```xml
<transport_descriptors>
    <transport_descriptor>
        <transport_id>UDPv4Transport</transport_id>
        <type>UDPv4</type>
        <receiveBufferSize>67108864</receiveBufferSize>  <!-- 64MB -->
    </transport_descriptor>
</transport_descriptors>

<participant profile_name="default_participant" is_default_profile="true">
    <rtps>
        <userTransports>
            <transport_id>UDPv4Transport</transport_id>
        </userTransports>
        <!-- useBuiltinTransports 默认为 true，或者被注释掉 -->
    </rtps>
</participant>
```

**实际运行时**：
```
Fast-DDS 会同时创建：
1. 内置的 UDPv4 传输
   - 缓冲区：208KB（系统默认）
   - 监听所有接口

2. 自定义的 UDPv4Transport
   - 缓冲区：64MB
   - 监听 interfaceWhiteList 中的接口

问题：
- Fast-DDS 在接收数据时可能选择内置的 UDPv4
- 导致使用 208KB 缓冲区
- 自定义的 64MB 配置不生效
- 仍然丢包
```

### 5.4 正确的配置

#### 完整配置示例

```xml
<?xml version="1.0" encoding="UTF-8"?>
<dds>
    <profiles xmlns="http://www.eprosima.com/XMLSchemas/fastRTPS_Profiles">
        <transport_descriptors>
            <!-- 自定义 UDP 传输 -->
            <transport_descriptor>
                <transport_id>UDPv4Transport</transport_id>
                <type>UDPv4</type>
                <interfaceWhiteList>
                    <address>192.168.0.100</address>
                </interfaceWhiteList>
                <sendBufferSize>67108864</sendBufferSize>      <!-- 64MB -->
                <receiveBufferSize>67108864</receiveBufferSize> <!-- 64MB -->
            </transport_descriptor>

            <!-- 自定义共享内存传输（用于本地通信） -->
            <transport_descriptor>
                <transport_id>SHMTransport</transport_id>
                <type>SHM</type>
            </transport_descriptor>
        </transport_descriptors>

        <participant profile_name="default_participant" is_default_profile="true">
            <rtps>
                <userTransports>
                    <transport_id>UDPv4Transport</transport_id>
                    <transport_id>SHMTransport</transport_id>
                </userTransports>
                <!-- 关键：必须设置为 false -->
                <useBuiltinTransports>false</useBuiltinTransports>
            </rtps>
        </participant>
    </profiles>
</dds>
```

#### 配置说明

**关键点**：
1. **必须设置 `useBuiltinTransports=false`**：
   - 禁用内置传输
   - 只使用自定义传输
   - 确保 64MB 缓冲区配置生效

2. **同时添加 SHMTransport**：
   - 保证本地进程间通信走共享内存
   - 不需要 127.0.0.1
   - 高效、零拷贝

3. **不需要 127.0.0.1**：
   - 共享内存不需要 IP 地址
   - 自动用于同一台机器的进程间通信
   - UDP 只用于远程通信

### 5.5 传输选择逻辑

**Fast-DDS 如何选择传输**：

```
发送数据时：
1. 检查目标节点的位置
   - 同一台机器？→ 使用 SHM（如果可用）
   - 不同机器？→ 使用 UDP

2. 如果 useBuiltinTransports=false
   - 只从 userTransports 中选择
   - 使用自定义配置（64MB 缓冲区）

3. 如果 useBuiltinTransports=true
   - 优先使用内置传输
   - 可能忽略自定义配置
   - 使用默认缓冲区（208KB）
```

**示例**：

```
Orin 上的节点 A 发布数据：
- 目标：Orin 上的节点 B（本地）
  → 使用 SHM（快速、零拷贝）

- 目标：用户电脑上的节点 C（远程）
  → 使用 UDP（64MB 缓冲区）
```

### 5.6 常见错误

#### 错误1：注释掉 useBuiltinTransports

```xml
<!-- <useBuiltinTransports>false</useBuiltinTransports> -->
```

**后果**：
- 默认值为 true
- 启用内置传输
- 自定义配置不生效

#### 错误2：设置为 true

```xml
<useBuiltinTransports>true</useBuiltinTransports>
```

**后果**：
- 同时使用内置和自定义传输
- 可能选择错误的传输
- 64MB 缓冲区不生效

#### 错误3：只配置 UDP，不配置 SHM

```xml
<userTransports>
    <transport_id>UDPv4Transport</transport_id>
    <!-- 缺少 SHMTransport -->
</userTransports>
<useBuiltinTransports>false</useBuiltinTransports>
```

**后果**：
- 本地通信也走 UDP
- 效率低下
- 占用网络带宽和缓冲区

### 5.7 验证配置是否生效

#### 方法1：检查 socket 缓冲区

```bash
# 启动 ROS2 节点后
sudo ss -u -a -n -m | grep -E "7400|7401" | grep skmem

# 期望输出：
# skmem:(r0,rb67108864,t0,tb67108864,...)
#           ^^^^^^^^^ 应该是 67108864（64MB）

# 如果是 rb212992（208KB），说明配置未生效
```

#### 方法2：监控 UDP 丢包

```bash
# 测试前
netstat -s | grep "receive buffer errors"

# 运行测试（30秒）
ros2 topic echo /livox/lidar_front --field header.stamp

# 测试后
netstat -s | grep "receive buffer errors"

# 如果数字不再增加，说明配置生效
```

### 5.8 总结

**为什么必须设置 useBuiltinTransports=false**：

1. **确保自定义配置生效**：
   - 64MB 缓冲区配置会被应用
   - interfaceWhiteList 会被遵守
   - 避免使用默认的 208KB 缓冲区

2. **避免传输冲突**：
   - 只使用自定义传输
   - 不会有内置和自定义传输的竞争
   - 传输选择逻辑清晰

3. **保持本地通信高效**：
   - 同时配置 SHMTransport
   - 本地通信走共享内存
   - 远程通信走 UDP（64MB 缓冲区）

**推荐配置**：
- 始终设置 `useBuiltinTransports=false`
- 同时配置 UDPv4Transport 和 SHMTransport
- 不需要 127.0.0.1
- 设置 receiveBufferSize 和 sendBufferSize 为 64MB

---

## 问题6：sendBufferSize、receiveBufferSize、sendSocketBufferSize、listenSocketBufferSize 的区别

### 6.1 参数概览

Fast-DDS 中有两套 API 用于配置 socket 缓冲区，它们之间存在映射关系：

| 旧 API（RTPSParticipantAttributes） | 新 API（SocketTransportDescriptor） | 作用域 |
|-----------------------------------|-----------------------------------|--------|
| sendSocketBufferSize | sendBufferSize | 发送缓冲区 |
| listenSocketBufferSize | receiveBufferSize | 接收缓冲区 |

**关键点**：
- 旧 API 是 RTPS 层的参数，会被自动映射到传输层
- 新 API 是传输描述符的参数，直接控制 socket 缓冲区
- 两者最终控制的是同一个东西：socket 的发送和接收缓冲区

### 6.2 sendSocketBufferSize 和 listenSocketBufferSize（旧 API）

#### 6.2.1 定义位置

```cpp
// include/fastdds/rtps/attributes/RTPSParticipantAttributes.h

struct RTPSParticipantAttributes
{
    /*! Send socket buffer size for the send resource.
     *  Zero value indicates to use default system buffer size.
     *  Default value: 0.
     */
    uint32_t sendSocketBufferSize = 0;

    /*! Listen socket buffer for all listen resources.
     *  Zero value indicates to use default system buffer size.
     *  Default value: 0.
     */
    uint32_t listenSocketBufferSize = 0;

    // ... 其他属性
};
```

#### 6.2.2 使用场景

**旧的 RTPS API**（已弃用，但仍然支持）：

```cpp
// C++ 代码示例
RTPSParticipantAttributes participant_attr;
participant_attr.sendSocketBufferSize = 16777216;    // 16MB
participant_attr.listenSocketBufferSize = 16777216;  // 16MB

RTPSParticipant* participant = RTPSDomain::createParticipant(
    domain_id,
    participant_attr
);
```

**XML 配置**：

```xml
<participant profile_name="default_participant">
    <rtps>
        <sendSocketBufferSize>16777216</sendSocketBufferSize>
        <listenSocketBufferSize>16777216</listenSocketBufferSize>
    </rtps>
</participant>
```

#### 6.2.3 映射关系（关键代码）

**证明：两组参数效果完全一样**

Fast-DDS 在创建传输描述符时，会将旧 API 的参数直接映射到新 API：

```cpp
// 文件：src/cpp/rtps/attributes/RTPSParticipantAttributes.cpp
// 行号：59-72

static std::shared_ptr<fastdds::rtps::UDPv4TransportDescriptor> create_udpv4_transport(
        const RTPSParticipantAttributes& att,
        bool intraprocess_only)
{
    auto descriptor = std::make_shared<fastdds::rtps::UDPv4TransportDescriptor>();

    // 关键映射：旧 API → 新 API
    descriptor->sendBufferSize = att.sendSocketBufferSize;        // ← 映射发送缓冲区
    descriptor->receiveBufferSize = att.listenSocketBufferSize;   // ← 映射接收缓冲区

    if (intraprocess_only)
    {
        // Avoid multicast leaving the host for intraprocess-only participants
        descriptor->TTL = 0;
    }
    return descriptor;
}
```

**同样的映射也存在于其他传输类型**：

```cpp
// UDPv6 传输（行号：74-87）
static std::shared_ptr<fastdds::rtps::UDPv6TransportDescriptor> create_udpv6_transport(
        const RTPSParticipantAttributes& att,
        bool intraprocess_only)
{
    auto descriptor = std::make_shared<fastdds::rtps::UDPv6TransportDescriptor>();
    descriptor->sendBufferSize = att.sendSocketBufferSize;        // ← 映射
    descriptor->receiveBufferSize = att.listenSocketBufferSize;   // ← 映射
    // ...
}

// TCPv4 传输（行号：89-104）
static std::shared_ptr<fastdds::rtps::TCPv4TransportDescriptor> create_tcpv4_transport(
        const RTPSParticipantAttributes& att)
{
    auto descriptor = std::make_shared<fastdds::rtps::TCPv4TransportDescriptor>();
    descriptor->add_listener_port(0);
    descriptor->sendBufferSize = att.sendSocketBufferSize;        // ← 映射
    descriptor->receiveBufferSize = att.listenSocketBufferSize;   // ← 映射
    // ...
}

// TCPv6 传输（行号：106-121）
static std::shared_ptr<fastdds::rtps::TCPv6TransportDescriptor> create_tcpv6_transport(
        const RTPSParticipantAttributes& att)
{
    auto descriptor = std::make_shared<fastdds::rtps::TCPv6TransportDescriptor>();
    descriptor->add_listener_port(0);
    descriptor->sendBufferSize = att.sendSocketBufferSize;        // ← 映射
    descriptor->receiveBufferSize = att.listenSocketBufferSize;   // ← 映射
    // ...
}
```

**映射流程**：

```
用户设置 sendSocketBufferSize = 16MB
    ↓
Fast-DDS 调用 create_udpv4_transport()
    ↓
descriptor->sendBufferSize = att.sendSocketBufferSize  // 直接赋值
    ↓
descriptor->sendBufferSize = 16MB
    ↓
最终设置 socket 的发送缓冲区为 16MB
```

**结论**：
- 旧 API（sendSocketBufferSize/listenSocketBufferSize）和新 API（sendBufferSize/receiveBufferSize）**效果完全一样**
- 旧 API 只是简单地赋值给新 API，没有任何额外处理
- 无论使用哪组参数，最终都是设置同一个 socket 缓冲区

### 6.3 sendBufferSize 和 receiveBufferSize（新 API）

#### 6.3.1 定义位置

```cpp
// include/fastdds/rtps/transport/SocketTransportDescriptor.h

struct SocketTransportDescriptor : public TransportDescriptorInterface
{
    //! Length of the send buffer.
    uint32_t sendBufferSize;

    //! Length of the receive buffer.
    uint32_t receiveBufferSize;

    // ... 其他属性
};
```

#### 6.3.2 使用场景

**新的传输描述符 API**（推荐）：

```cpp
// C++ 代码示例
auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();
udp_transport->sendBufferSize = 16777216;      // 16MB
udp_transport->receiveBufferSize = 16777216;   // 16MB

DomainParticipantQos qos;
qos.transport().user_transports.push_back(udp_transport);
qos.transport().use_builtin_transports = false;

DomainParticipant* participant = DomainParticipantFactory::get_instance()
    ->create_participant(domain_id, qos);
```

**XML 配置**：

```xml
<transport_descriptors>
    <transport_descriptor>
        <transport_id>UDPv4Transport</transport_id>
        <type>UDPv4</type>
        <sendBufferSize>16777216</sendBufferSize>
        <receiveBufferSize>16777216</receiveBufferSize>
    </transport_descriptor>
</transport_descriptors>
```

#### 6.3.3 实际使用

```cpp
// src/cpp/rtps/transport/UDPTransportInterface.cpp

bool UDPTransportInterface::init(const PropertyPolicy*)
{
    // 如果用户设置了 sendBufferSize 或 receiveBufferSize
    if (configuration()->sendBufferSize == 0 || configuration()->receiveBufferSize == 0)
    {
        // 获取系统默认的 socket 缓冲区大小
        ip::udp::socket socket(io_service_);
        socket.open(generate_protocol());

        if (configuration()->sendBufferSize == 0)
        {
            socket_base::send_buffer_size option;
            socket.get_option(option);
            set_send_buffer_size(static_cast<uint32_t>(option.value()));

            // 如果系统默认值 < 64KB，强制设置为 64KB
            if (configuration()->sendBufferSize < s_minimumSocketBuffer)
            {
                set_send_buffer_size(s_minimumSocketBuffer);  // 64KB
                mSendBufferSize = s_minimumSocketBuffer;
            }
        }

        if (configuration()->receiveBufferSize == 0)
        {
            socket_base::receive_buffer_size option;
            socket.get_option(option);
            set_receive_buffer_size(static_cast<uint32_t>(option.value()));

            // 如果系统默认值 < 64KB，强制设置为 64KB
            if (configuration()->receiveBufferSize < s_minimumSocketBuffer)
            {
                set_receive_buffer_size(s_minimumSocketBuffer);  // 64KB
                mReceiveBufferSize = s_minimumSocketBuffer;
            }
        }
    }

    return true;
}
```

### 6.4 四个参数的对比总结

| 参数 | API 类型 | 定义位置 | 作用 | 推荐使用 |
|------|---------|---------|------|---------|
| sendSocketBufferSize | 旧 API | RTPSParticipantAttributes | 发送缓冲区（会被映射到 sendBufferSize） | ❌ 已弃用 |
| listenSocketBufferSize | 旧 API | RTPSParticipantAttributes | 接收缓冲区（会被映射到 receiveBufferSize） | ❌ 已弃用 |
| sendBufferSize | 新 API | SocketTransportDescriptor | 发送缓冲区（直接控制 socket） | ✅ 推荐 |
| receiveBufferSize | 新 API | SocketTransportDescriptor | 接收缓冲区（直接控制 socket） | ✅ 推荐 |

### 6.5 初始化逻辑详解

#### 6.5.1 receiveBufferSize 的初始化

```cpp
// UDPTransportInterface.cpp init() 方法

if (configuration()->receiveBufferSize == 0)
{
    // 步骤1：获取系统当前的 socket 默认缓冲区大小
    socket_base::receive_buffer_size option;
    socket.get_option(option);
    uint32_t system_default = option.value();

    // 步骤2：如果系统默认值 < 64KB，强制设置为 64KB
    if (system_default < 65536)
    {
        mReceiveBufferSize = 65536;  // 64KB
    }
    else
    {
        mReceiveBufferSize = 0;  // 保持为 0，表示使用系统默认值
    }
}
else
{
    // 用户配置了 receiveBufferSize，直接使用
    mReceiveBufferSize = configuration()->receiveBufferSize;
}
```

**初始化流程图**：

```mermaid
flowchart TD
    A[开始初始化]
    B{receiveBufferSize == 0?}
    C[获取系统默认缓冲区大小]
    D{系统默认 < 64KB?}
    E[设置为 64KB]
    F[保持为 0<br/>使用系统默认]
    G[使用用户配置的值]
    H[创建 socket 时<br/>调用 set_option]
    I[创建 socket 时<br/>不调用 set_option]

    A --> B
    B -->|是| C
    B -->|否| G
    C --> D
    D -->|是| E
    D -->|否| F
    E --> H
    F --> I
    G --> H

    style E fill:#ffebee,stroke:#c62828,stroke-width:2px
    style F fill:#e8f5e9,stroke:#1b5e20,stroke-width:2px
    style G fill:#e1f5ff,stroke:#01579b,stroke-width:2px
```

#### 6.5.2 创建 socket 时的处理

```cpp
// UDPv4Transport.cpp OpenInputChannel() 方法

bool UDPv4Transport::OpenInputChannel(const Locator& locator)
{
    // 创建 UDP socket
    std::shared_ptr<asio::ip::udp::socket> socket =
        std::make_shared<asio::ip::udp::socket>(io_service_);

    socket->open(asio::ip::udp::v4());

    // 如果 mReceiveBufferSize != 0，设置接收缓冲区
    if (mReceiveBufferSize != 0)
    {
        socket_base::receive_buffer_size option(mReceiveBufferSize);
        socket->set_option(option);

        // 实际缓冲区大小 = min(mReceiveBufferSize, net.core.rmem_max)
    }
    // 如果 mReceiveBufferSize == 0，不调用 set_option()，使用系统默认值

    // 绑定端口
    socket->bind(endpoint);

    return true;
}
```

**关键点**：
- 如果 `mReceiveBufferSize != 0`，调用 `set_option()` 设置缓冲区
- 如果 `mReceiveBufferSize == 0`，不调用 `set_option()`，使用系统默认值
- 实际缓冲区大小受系统限制：`min(请求值, net.core.rmem_max)`

### 6.6 实际案例分析

#### 案例1：用户电脑（系统默认缓冲区很小）

**场景**：
```bash
# 系统配置
net.core.rmem_max = 212992  # 208KB
net.core.rmem_default = 8192  # 8KB（假设）
```

**情况1：不配置 receiveBufferSize**

```xml
<!-- 不配置，或者配置为 0 -->
<receiveBufferSize>0</receiveBufferSize>
```

```
初始化流程：
1. receiveBufferSize == 0
2. 获取系统默认值 = 8KB
3. 8KB < 64KB，设置 mReceiveBufferSize = 64KB
4. 创建 socket 时调用 set_option(64KB)
5. 实际缓冲区 = min(64KB, 208KB) = 64KB

结果：64KB（不够用，会丢包）
```

**情况2：配置 receiveBufferSize = 16MB**

```xml
<receiveBufferSize>16777216</receiveBufferSize>
```

```
初始化流程：
1. receiveBufferSize == 16MB
2. 跳过自动调整
3. mReceiveBufferSize = 16MB
4. 创建 socket 时调用 set_option(16MB)
5. 实际缓冲区 = min(16MB, 208KB) = 208KB

结果：208KB（比 64KB 好 3 倍）
```

**情况3：增加系统限制 + 配置 receiveBufferSize**

```bash
# 增加系统限制
sudo sysctl -w net.core.rmem_max=67108864  # 64MB
```

```xml
<receiveBufferSize>67108864</receiveBufferSize>
```

```
初始化流程：
1. receiveBufferSize == 64MB
2. 跳过自动调整
3. mReceiveBufferSize = 64MB
4. 创建 socket 时调用 set_option(64MB)
5. 实际缓冲区 = min(64MB, 64MB) = 64MB

结果：64MB（足够用，不会丢包）
```

#### 案例2：x86（系统默认缓冲区已经足够）

**场景**：
```bash
# 系统配置
net.core.rmem_max = 212992  # 208KB
net.core.rmem_default = 212992  # 208KB（假设）
```

**情况1：不配置 receiveBufferSize**

```xml
<receiveBufferSize>0</receiveBufferSize>
```

```
初始化流程：
1. receiveBufferSize == 0
2. 获取系统默认值 = 208KB
3. 208KB >= 64KB，保持 mReceiveBufferSize = 0
4. 创建 socket 时不调用 set_option()
5. 实际缓冲区 = 系统默认值 = 208KB

结果：208KB（勉强够用）
```

### 6.7 为什么需要两套 API

#### 历史原因

**旧 API（sendSocketBufferSize / listenSocketBufferSize）**：
- Fast-RTPS 1.x 时代的 API
- 在 RTPS 层配置，自动应用到所有传输
- 简单但不够灵活

**新 API（sendBufferSize / receiveBufferSize）**：
- Fast-DDS 2.x 引入的 API
- 在传输描述符层配置，可以为不同传输设置不同的缓冲区
- 更灵活，更强大

#### 兼容性

```cpp
// Fast-DDS 内部会自动处理兼容性
if (participant_attr.sendSocketBufferSize != 0)
{
    // 如果用户使用旧 API，自动映射到新 API
    descriptor->sendBufferSize = participant_attr.sendSocketBufferSize;
}
```

**推荐做法**：
- 新项目：使用新 API（sendBufferSize / receiveBufferSize）
- 旧项目：可以继续使用旧 API，但建议迁移到新 API

### 6.8 配置建议

#### 推荐配置（新 API）

```xml
<?xml version="1.0" encoding="UTF-8"?>
<dds>
    <profiles xmlns="http://www.eprosima.com/XMLSchemas/fastRTPS_Profiles">
        <transport_descriptors>
            <transport_descriptor>
                <transport_id>UDPv4Transport</transport_id>
                <type>UDPv4</type>
                <interfaceWhiteList>
                    <address>192.168.0.100</address>
                </interfaceWhiteList>
                <!-- 使用新 API -->
                <sendBufferSize>67108864</sendBufferSize>      <!-- 64MB -->
                <receiveBufferSize>67108864</receiveBufferSize> <!-- 64MB -->
            </transport_descriptor>
            <transport_descriptor>
                <transport_id>SHMTransport</transport_id>
                <type>SHM</type>
            </transport_descriptor>
        </transport_descriptors>
        <participant profile_name="default_participant" is_default_profile="true">
            <rtps>
                <userTransports>
                    <transport_id>UDPv4Transport</transport_id>
                    <transport_id>SHMTransport</transport_id>
                </userTransports>
                <useBuiltinTransports>false</useBuiltinTransports>
            </rtps>
        </participant>
    </profiles>
</dds>
```

#### 不推荐配置（旧 API）

```xml
<participant profile_name="default_participant">
    <rtps>
        <!-- 不推荐：使用旧 API -->
        <sendSocketBufferSize>67108864</sendSocketBufferSize>
        <listenSocketBufferSize>67108864</listenSocketBufferSize>
    </rtps>
</participant>
```

**为什么不推荐**：
- 旧 API 已弃用
- 不够灵活（无法为不同传输设置不同的缓冲区）
- 可能在未来版本中移除

### 6.9 验证配置是否生效

```bash
# 1. 启动 ROS2 节点后，查看实际的 socket 缓冲区大小
sudo ss -u -a -n -m | grep -E "7400|7401" | grep skmem

# 期望输出：
# skmem:(r0,rb67108864,t0,tb67108864,f0,w0,o0,bl0,d0)
#           ^^^^^^^^^ 接收缓冲区应该是 67108864（64MB）
#                         ^^^^^^^^^ 发送缓冲区应该是 67108864（64MB）

# 如果是 rb212992（208KB），说明配置未生效
```

**可能的原因**：
1. 没有设置 `useBuiltinTransports=false`
2. 系统限制 `net.core.rmem_max` 太小
3. XML 配置文件路径错误

### 6.10 总结

**核心结论：两组参数效果完全一样**

```cpp
// 证据：src/cpp/rtps/attributes/RTPSParticipantAttributes.cpp:64-65
descriptor->sendBufferSize = att.sendSocketBufferSize;        // 直接赋值
descriptor->receiveBufferSize = att.listenSocketBufferSize;   // 直接赋值
```

**四个参数的关系**：

```
旧 API（RTPS 层）                新 API（传输层）              实际效果
sendSocketBufferSize    →    sendBufferSize       →    socket 发送缓冲区
listenSocketBufferSize  →    receiveBufferSize    →    socket 接收缓冲区
                             (直接赋值，无额外处理)
```

**配置等价性示例**：

```xml
<!-- 配置方式1：使用旧 API -->
<participant profile_name="default_participant">
    <rtps>
        <sendSocketBufferSize>67108864</sendSocketBufferSize>
        <listenSocketBufferSize>67108864</listenSocketBufferSize>
    </rtps>
</participant>

<!-- 配置方式2：使用新 API -->
<transport_descriptors>
    <transport_descriptor>
        <transport_id>UDPv4Transport</transport_id>
        <type>UDPv4</type>
        <sendBufferSize>67108864</sendBufferSize>
        <receiveBufferSize>67108864</receiveBufferSize>
    </transport_descriptor>
</transport_descriptors>

<!-- 两种配置方式最终效果完全一样！ -->
```

**关键点**：

1. **两组参数控制同一个东西**：
   - 旧 API 会被直接赋值给新 API（见代码第 64-65 行）
   - 最终都是设置 socket 的发送和接收缓冲区
   - **没有任何额外的转换或处理**

2. **为什么推荐使用新 API**：
   - 虽然效果一样，但新 API 更灵活
   - 可以为不同传输设置不同的缓冲区
   - 旧 API 已标记为弃用，可能在未来版本中移除

3. **初始化逻辑**：
   - 如果配置为 0，使用系统默认值（但如果 < 64KB，强制设置为 64KB）
   - 如果配置了具体值，调用 `set_option()` 设置
   - 实际缓冲区大小受系统限制：`min(配置值, net.core.rmem_max)`

4. **配置建议**：
   - 新项目：使用新 API（sendBufferSize / receiveBufferSize）
   - 旧项目：可以继续使用旧 API，但建议迁移
   - 设置为 64MB（67108864）
   - 同时增加系统限制 `net.core.rmem_max`
   - 必须设置 `useBuiltinTransports=false`（使用新 API 时）

5. **验证方法**：
   - 使用 `ss -u -a -n -m` 查看实际的 socket 缓冲区大小
   - 监控 UDP 丢包统计
   - 测试接收频率是否稳定

**代码位置总结**：

| 代码位置 | 说明 |
|---------|------|
| `src/cpp/rtps/attributes/RTPSParticipantAttributes.cpp:64-65` | 旧 API → 新 API 的映射（UDPv4） |
| `src/cpp/rtps/attributes/RTPSParticipantAttributes.cpp:79-80` | 旧 API → 新 API 的映射（UDPv6） |
| `src/cpp/rtps/attributes/RTPSParticipantAttributes.cpp:94-95` | 旧 API → 新 API 的映射（TCPv4） |
| `src/cpp/rtps/attributes/RTPSParticipantAttributes.cpp:111-112` | 旧 API → 新 API 的映射（TCPv6） |
| `src/cpp/rtps/transport/UDPTransportInterface.cpp:122-153` | 缓冲区初始化逻辑 |
| `include/fastdds/rtps/attributes/RTPSParticipantAttributes.h:491-496` | 旧 API 的定义 |
| `include/fastdds/rtps/transport/SocketTransportDescriptor.h:85-87` | 新 API 的定义 |

## 问题7：接收率低是系统丢了 UDP 包，还是应用程序丢了消息？

### 问题背景

用户电脑订阅 Orin 的雷达数据（/livox/lidar_front），发现接收率只有 43%：
- 预期：10Hz × 30秒 = 300 帧
- 实际：只接收到 129 帧
- 丢失：171 帧

需要确定丢包发生在哪一层：
1. **UDP 层**：网络传输丢包（系统层面）
2. **应用层**：Fast-DDS 丢消息（应用层面）

### 问题回答

**答案：应用程序（Fast-DDS）丢了消息，不是系统丢的 UDP 包。**

> 注：分析过程中经历了多次假设和推翻。以下按时间顺序记录证据和分析过程。

### 证据收集

#### 证据1：UDP 层丢包很少

```bash
# 测试前
BEFORE=$(netstat -s | grep "receive buffer errors" | awk '{print $1}')

# 订阅 30 秒
ros2 topic echo /livox/lidar_front

# 测试后
AFTER=$(netstat -s | grep "receive buffer errors" | awk '{print $1}')
UDP_LOSS=$((AFTER - BEFORE))
```

**结果**：
- UDP 丢包：17 个包 / 30 秒
- 每帧数据：354 个 UDP 包（0.5MB ÷ 1472 字节/包）
- UDP 丢包影响：17 ÷ 354 ≈ **0.05 帧**
- **但实际丢失了 171 帧！** → 丢包发生在应用层

#### 证据2：QoS 对比测试

| 发布者 QoS | 订阅者配置 | 接收率 | 说明 |
|-----------|-----------|--------|------|
| RELIABLE | 无配置 | 2% | ACK 机制严重影响性能 |
| RELIABLE | BEST_EFFORT | 50% | QoS 不匹配，性能差 |
| BEST_EFFORT | 无配置 | 96% | 最佳配置 |
| BEST_EFFORT | interfaceWhiteList | 43% | 白名单导致接收率下降 |

#### 证据3：Socket 状态（使用白名单时）

```bash
# 查看 UDP socket
ss -u -a -n | grep "13650"
```

**结果**：有 4 个 socket 绑定到端口 13650：

```
UNCONN 0  0  239.255.0.1:13650    0.0.0.0:*
UNCONN 0  0  192.168.0.100:13650  0.0.0.0:*
UNCONN 0  0  239.255.0.1:13650    0.0.0.0:*
UNCONN 0  0  192.168.0.100:13650  0.0.0.0:*
```

Socket 内存统计：

```bash
ss -u -a -n -m | grep -A 1 "13650"
```

```
skmem:(r0,rb134217728,t0,tb212992,f4096,w0,o48,bl0,d0)
```

- `r0`：接收队列为空（数据被及时处理）
- `rb134217728`：接收缓冲区 = 128MB（配置的 64MB × 2）
- 缓冲区没有满，不是缓冲区不足的问题

#### 证据4：接口统计

```bash
# 测试前后对比
ip -s link show enp4s0 | grep -A 1 "RX:"
ip -s link show enx00e04c6808cc | grep -A 1 "RX:"
```

**结果**：
- enp4s0 (192.168.0.100)：多播包增加 223 个 ✓
- enx00e04c6808cc (10.11.12.144)：多播包增加 0 个
- 多播包只从 enp4s0 接口到达

#### 证据5：绑定地址与多播接收测试

```python
# 同时创建两个 socket 监听 13650 端口
Socket A: bind(192.168.0.100:13650) + join(239.255.0.1)
Socket B: bind(239.255.0.1:13650)   + join(239.255.0.1)
```

**结果**：

```
Socket A (192.168.0.100):   0 包    ← 完全收不到多播包！
Socket B (239.255.0.1):   121 包    ← 只有它能收到
```

**结论**：在 Linux 上，绑定到特定单播地址的 socket，即使加入了多播组，也**无法接收多播包**。

#### 证据6：抓包分析 - 发现数据走单播

```bash
# 抓取 13650 端口（发现协议）
sudo tcpdump -i enp4s0 -n -c 10 'udp and port 13650'
```

```
192.168.0.10.37700 > 239.255.0.1.13650: UDP, length 252    ← 发现协议，多播，小包
```

```bash
# 抓取大包（雷达数据）
sudo tcpdump -i enp4s0 -n -c 5 \
  'udp and src host 192.168.0.11 and dst host 192.168.0.100 and greater 1000'
```

```
192.168.0.11.45846 > 192.168.0.100.13661: UDP, length 65468    ← 雷达数据，单播，大包
```

**关键发现**：雷达数据通过**单播**发送到 `192.168.0.100:13661`，不是多播！

#### 证据7：端口分布统计

```bash
sudo timeout 5 tcpdump -i enp4s0 -n \
  'udp and src host 192.168.0.11 and dst host 192.168.0.100' 2>&1 | \
  grep -oP '192\.168\.0\.100\.\d+' | sort | uniq -c | sort -rn
```

```
384 192.168.0.100.13661    ← daemon 的 user data 端口（participant 0）
136 192.168.0.100.13662    ← participant 1 的 metatraffic 端口
136 192.168.0.100.13660    ← daemon 的 metatraffic 端口（participant 0）
  0 192.168.0.100.13665    ← 订阅者的 user data 端口（participant 2）⚠️
```

#### 证据8：丢包模式 - 周期性断流

```
使用白名单（60秒测试）：
  5.0s: 48/50 帧 ( 96.0%)
 10.0s: 44/50 帧 ( 88.0%)
 15.0s: 48/50 帧 ( 96.0%)
 20.0s: 36/50 帧 ( 72.0%)   ← 开始下降
 25.0s:  0/50 帧 (  0.0%)   ← 完全断流
 30.1s:  0/50 帧 (  0.0%)   ← 持续断流
 35.1s: 28/50 帧 ( 56.0%)   ← 恢复
 45.1s: 47/50 帧 ( 94.0%)   ← 恢复正常
 55.1s:  0/50 帧 (  0.0%)   ← 又断流
```

- 不使用白名单时也有断流，但更晚出现（55 秒才断流）
- 断流期间 Orin 上仍能看到订阅者（Subscription count: 1）
- 小消息（IMU 584 字节）不丢包，大消息（雷达 0.5MB）丢包严重

### 分析过程

#### 假设1：多播绑定机制问题（已推翻）

**假设**：interfaceWhiteList 导致 socket 绑定到 192.168.0.100，无法接收多播包，因此丢包。

**源码依据**（`UDPv4Transport.cpp`）：

```cpp
// get_binding_interfaces_list() 第 431-447 行
if (is_interface_whitelist_empty())
    vOutputInterfaces.push_back(s_IPv4AddressAny);  // 绑定到 0.0.0.0
else
    for (auto& ip : interface_whitelist_)
        vOutputInterfaces.push_back(ip.to_string());  // 绑定到 192.168.0.100
```

**推翻原因**：tcpdump 证明雷达数据走**单播**（证据6），不走多播端口 13650。绑定到 192.168.0.100 的 socket 可以正常接收单播包。

#### 假设2：多 socket 导致包分散（已推翻）

**假设**：4 个 socket 绑定到同一端口 13650，Linux 内核随机分配多播包，导致每个 socket 只收到部分包，大消息重组失败。

**推翻原因**：雷达数据走单播端口 13661（证据7），不走多播端口 13650。多 socket 问题只影响发现协议的多播包，不影响数据传输。

#### 假设3：修改 Fast-DDS 源码去掉多余 socket（已推翻）

**尝试**：修改 `UDPv4Transport.cpp` 的 `OpenInputChannel()`，不创建额外的多播 socket。

**推翻原因**：问题不在多播 socket，而在单播数据传输路径。修改已回退（`git checkout`）。

### 根因（已确认）

**根因：Linux 内核 IP fragment 重组缓冲区溢出，触发雪崩式丢包。**

#### 数据传输链路

```
Orin 发送雷达数据（10Hz，0.5MB/帧）
    ↓ RTPS 分片：每帧 → 8 个 DATA_FRAG（fragment_size = 65404 字节）
    ↓ IP 分片：每个 65KB UDP 包 → ~45 个 IP fragment（MTU=1500）
    ↓ 网络传输：10Hz × 8 × 45 ≈ 3600 IP fragments/sec ≈ 5MB/s
    ↓ 用户电脑 NIC 接收（NIC_drop=0，网卡层无丢包）
    ↓ Linux 内核 IP fragment 重组 ← ⚠️ 瓶颈在这里
    ↓ UDP socket 交付
    ↓ Fast-DDS 应用层处理
```

#### 为什么 IP 重组会失败

Linux 内核默认参数：

| 参数 | 默认值 | 含义 |
|------|--------|------|
| `ipfrag_high_thresh` | 4MB | IP fragment 重组缓冲区上限 |
| `ipfrag_low_thresh` | 3MB | 缓冲区清理低水位 |
| `ipfrag_time` | 30s | fragment 超时时间 |

检查当前系统参数：

```bash
# 查看 IP fragment 重组相关内核参数
cat /proc/sys/net/ipv4/ipfrag_high_thresh   # 缓冲区上限（字节）
cat /proc/sys/net/ipv4/ipfrag_low_thresh    # 缓冲区低水位（字节）
cat /proc/sys/net/ipv4/ipfrag_time          # fragment 超时时间（秒）
cat /proc/sys/net/ipv4/ipfrag_max_dist      # 最大乱序距离
```

查看当前 IP 重组统计（实时增量）：

```bash
# 查看 /proc/net/snmp 中的 IP 重组计数器
# ReasmReqds: 收到需要重组的 IP fragment 数
# ReasmOKs:   重组成功的 UDP 包数
# ReasmFails: 重组失败的次数（关键指标！）
cat /proc/net/snmp | grep -A1 "^Ip:"
```

综合监控脚本（同时追踪 NIC、softnet、UDP、IP 重组各层统计）：

```bash
# 监控脚本位于 /tmp/monitor_all_layers.py
# 用法：在一个终端运行监控，另一个终端运行 subscriber
python3 /tmp/monitor_all_layers.py 60 enp4s0
```

检查 RTPS 端口 socket 绑定情况：

```bash
# 查看 RTPS 相关端口的 socket 绑定（Domain ID=25 的端口范围 13650-13670）
ss -anup | grep -E ':136[5-7][0-9]\b'
```

问题链：

1. 每秒 ~5MB 的 IP fragment 流量持续涌入
2. 网络正常波动导致少量 fragment 丢失，对应的 UDP 包无法完成重组
3. 这些不完整的 fragment 堆积在内核重组缓冲区中，等待 30 秒超时
4. 约 30 秒后，缓冲区累积达到 4MB 上限（`ipfrag_high_thresh`）
5. 内核触发**全量清理**：丢弃所有正在重组的 fragment，直到降到 3MB（`ipfrag_low_thresh`）
6. 清理期间，新到的 fragment 也被丢弃（因为它们依赖的前序 fragment 已被清除）
7. 形成**雪崩效应**：`ReasmOK=0, ReasmFail=3500/s`，100% 重组失败，持续 10-15 秒
8. 清理完成后恢复正常，开始下一个周期

#### 证据9：综合监控数据（关键证据）

同时运行网络各层监控脚本和 subscriber，精确定位丢包层级。

监控脚本 `monitor_all_layers.py`：

```python
#!/usr/bin/env python3
"""
综合监控脚本：同时追踪网络各层统计，定位丢包层级
用法: python3 monitor_all_layers.py [duration_seconds] [interface]
"""
import time, sys, os, subprocess, re

duration = int(sys.argv[1]) if len(sys.argv) > 1 else 60
iface = sys.argv[2] if len(sys.argv) > 2 else "enp4s0"

def read_file(path):
    try:
        with open(path) as f:
            return f.read()
    except:
        return ""

def get_snmp_udp():
    """从 /proc/net/snmp 获取 UDP 和 IP 统计"""
    content = read_file("/proc/net/snmp")
    result = {}
    lines = content.strip().split('\n')
    for i in range(0, len(lines)-1, 2):
        if lines[i].startswith("Udp:") or lines[i].startswith("Ip:"):
            keys = lines[i].split()
            vals = lines[i+1].split()
            prefix = "Udp_" if lines[i].startswith("Udp:") else "Ip_"
            for k, v in zip(keys[1:], vals[1:]):
                result[f"{prefix}{k}"] = int(v)
    return result

def get_nic_stats():
    """从 /sys/class/net 获取 NIC 统计"""
    base = f"/sys/class/net/{iface}/statistics"
    result = {}
    for name in ["rx_packets", "rx_bytes", "rx_dropped", "rx_errors", "rx_fifo_errors"]:
        result[f"NIC_{name}"] = int(read_file(f"{base}/{name}").strip() or "0")
    return result

def get_softnet():
    """从 /proc/net/softnet_stat 获取软中断统计"""
    content = read_file("/proc/net/softnet_stat")
    total_processed = total_dropped = total_squeezed = 0
    for line in content.strip().split('\n'):
        parts = line.split()
        if len(parts) >= 3:
            total_processed += int(parts[0], 16)
            total_dropped += int(parts[1], 16)
            total_squeezed += int(parts[2], 16)
    return {"softnet_processed": total_processed, "softnet_dropped": total_dropped,
            "softnet_squeezed": total_squeezed}

def get_socket_info():
    """获取 RTPS 相关 socket 信息"""
    try:
        out = subprocess.check_output(["ss", "-anup"], text=True, timeout=2)
        return [l.strip() for l in out.split('\n') if re.search(r':1366[0-9]', l)]
    except:
        return []

# 初始打印 socket 信息
print("=" * 100)
print(f"监控开始: 接口={iface}, 持续={duration}s")
print("=" * 100)
socks = get_socket_info()
if socks:
    print(f"\n当前 RTPS 端口 socket ({len(socks)} 个):")
    for s in socks:
        print(f"  {s}")
else:
    print("\n当前无 RTPS 端口 socket (ROS2 未运行)")
print()

header = (f"{'时间':>4s} | {'NIC_rx':>8s} {'NIC_MB/s':>8s} {'NIC_drop':>8s} | "
          f"{'softDrop':>8s} {'squeeze':>8s} | "
          f"{'UdpIn':>7s} {'UdpErr':>7s} {'RcvBuf':>7s} | "
          f"{'ReasmReq':>8s} {'ReasmOK':>8s} {'ReasmFl':>8s} | "
          f"{'InDeliv':>8s}")
print(header)
print("-" * len(header))

prev = {**get_snmp_udp(), **get_nic_stats(), **get_softnet()}
time.sleep(1)

for sec in range(1, duration + 1):
    curr = {**get_snmp_udp(), **get_nic_stats(), **get_softnet()}
    d = {k: curr.get(k, 0) - prev.get(k, 0) for k in curr}

    nic_rx = d.get("NIC_rx_packets", 0)
    nic_mb = d.get("NIC_rx_bytes", 0) / 1024 / 1024
    nic_drop = d.get("NIC_rx_dropped", 0)
    soft_drop = d.get("softnet_dropped", 0)
    soft_squeeze = d.get("softnet_squeezed", 0)
    udp_in = d.get("Udp_InDatagrams", 0)
    udp_err = d.get("Udp_InErrors", 0)
    rcv_buf = d.get("Udp_RcvbufErrors", 0)
    reasm_req = d.get("Ip_ReasmReqds", 0)
    reasm_ok = d.get("Ip_ReasmOKs", 0)
    reasm_fail = d.get("Ip_ReasmFails", 0)
    in_deliv = d.get("Ip_InDelivers", 0)

    flags = ""
    if nic_drop > 0: flags += " NIC_DROP!"
    if soft_drop > 0: flags += " SOFT_DROP!"
    if soft_squeeze > 0: flags += " SQUEEZE!"
    if udp_err > 0: flags += " UDP_ERR!"
    if rcv_buf > 0: flags += " RCVBUF!"
    if reasm_fail > 0: flags += " REASM_FAIL!"

    print(f"{sec:4d} | {nic_rx:8d} {nic_mb:8.2f} {nic_drop:8d} | "
          f"{soft_drop:8d} {soft_squeeze:8d} | "
          f"{udp_in:7d} {udp_err:7d} {rcv_buf:7d} | "
          f"{reasm_req:8d} {reasm_ok:8d} {reasm_fail:8d} | "
          f"{in_deliv:8d}{flags}")

    prev = curr
    time.sleep(1)

print("\n" + "=" * 100)
print("监控结束，当前 socket 状态:")
for s in get_socket_info():
    print(f"  {s}")
print("=" * 100)
```

运行方式（需要两个终端）：

```bash
# 终端1：启动监控
python3 monitor_all_layers.py 60 enp4s0

# 终端2：启动 subscriber（监控启动几秒后再运行）
export FASTRTPS_DEFAULT_PROFILES_FILE=~/.ros/fastdds_profiles.xml
ros2 topic hz /livox/lidar_front --window 50
```

监控输出中各列含义：

| 列名 | 数据来源 | 含义 |
|------|---------|------|
| NIC_rx | `/sys/class/net/<iface>/statistics/rx_packets` | 网卡每秒收包数 |
| NIC_MB/s | `/sys/class/net/<iface>/statistics/rx_bytes` | 网卡每秒收字节数 |
| NIC_drop | `/sys/class/net/<iface>/statistics/rx_dropped` | 网卡丢包数 |
| softDrop | `/proc/net/softnet_stat` 第2列 | 软中断丢包数 |
| squeeze | `/proc/net/softnet_stat` 第3列 | 软中断时间片不足次数 |
| UdpIn | `/proc/net/snmp` Udp InDatagrams | UDP 层收到的完整数据报数 |
| UdpErr | `/proc/net/snmp` Udp InErrors | UDP 层错误数 |
| RcvBuf | `/proc/net/snmp` Udp RcvbufErrors | UDP 接收缓冲区溢出次数 |
| ReasmReq | `/proc/net/snmp` Ip ReasmReqds | 需要 IP 重组的 fragment 数 |
| ReasmOK | `/proc/net/snmp` Ip ReasmOKs | IP 重组成功的数据报数 |
| ReasmFl | `/proc/net/snmp` Ip ReasmFails | IP 重组失败次数（**关键指标**） |

监控结果（修复前，`ipfrag_high_thresh=4MB`）：

```
  时间 | NIC_rx NIC_MB/s NIC_drop | UdpIn UdpErr RcvBuf | ReasmReq ReasmOK ReasmFl
------+-------------------------+---------------------+-------------------------
  正常阶段（第13-29秒）：
  13  |   4810    2.50        0 |   4326      0      0 |     1194       20       0
  14  |   3357    4.57        0 |    281      0      0 |     3197       66       0
  ...
  29  |   3735    5.10        0 |    287      0      0 |     3578       80       0
------+-------------------------+---------------------+-------------------------
  丢包阶段（第30-42秒）：
  30  |   3599    4.86        0 |    303      0      0 |     3397       33    1859  ← 重组开始失败！
  31  |   3723    5.07        0 |    224      0      0 |     3549        0    3549  ← 100% 失败
  32  |   3736    5.10        0 |    212      0      0 |     3578        0    3578  ← 100% 失败
  ...
  42  |   3671    4.95        0 |    272      0      0 |     3464        0    3464  ← 100% 失败
------+-------------------------+---------------------+-------------------------
  恢复阶段（第43-44秒）：
  43  |   3750    5.10        0 |    238      0      0 |     3576       24    2516  ← 开始恢复
  44  |   3559    4.85        0 |    280      0      0 |     3401       73       7  ← 恢复正常
```

关键观察：
- NIC 层全程收包正常（3500-3700 pkts/sec），NIC_drop=0
- 丢包阶段 `ReasmOK=0`（无一个 UDP 包能完成重组），`ReasmFail=3500/s`（全部失败）
- `UdpIn` 从正常的 ~300-600 降到 ~200-280（因为重组失败，UDP 包无法交付）
- 丢包持续约 13 秒，与 `ipfrag_time=30s` 的超时周期吻合

#### 排除的其他假设

| 假设 | 排除原因 |
|------|---------|
| DDS lease duration 超时 | 设置 `DURATION_INFINITY` 仍然丢包 |
| UDP socket 缓冲区溢出 | `RcvbufErrors=0`，`Recv-Q=0` |
| NIC 网卡丢包 | `NIC_drop=0` |
| softnet 软中断丢包 | `softnet_dropped=0` |
| Fast-DDS 应用层逻辑 | 数据根本没到达 UDP socket（IP 重组就失败了） |

### 解决方案

增大内核 IP fragment 重组缓冲区，缩短超时时间：

```bash
# 临时生效
sudo sysctl -w net.ipv4.ipfrag_high_thresh=67108864   # 64MB（默认 4MB）
sudo sysctl -w net.ipv4.ipfrag_low_thresh=50331648     # 48MB（默认 3MB）
sudo sysctl -w net.ipv4.ipfrag_time=5                   # 5秒（默认 30秒）
```

```bash
# 永久生效（重启后保持）
sudo tee /etc/sysctl.d/99-ipfrag.conf << 'EOF'
# Fix IP fragment reassembly for large ROS2/DDS traffic
net.ipv4.ipfrag_high_thresh = 67108864
net.ipv4.ipfrag_low_thresh = 50331648
net.ipv4.ipfrag_time = 5
EOF
sudo sysctl -p /etc/sysctl.d/99-ipfrag.conf
```

参数说明：
- `ipfrag_high_thresh=64MB`：缓冲区上限从 4MB 增大到 64MB，避免溢出触发全量清理
- `ipfrag_low_thresh=48MB`：低水位相应增大
- `ipfrag_time=5s`：超时从 30 秒缩短到 5 秒，不完整的 fragment 更快释放，减少缓冲区占用

### 修复验证

修复后同样运行 50 秒监控：

```
修复前（ipfrag_high_thresh=4MB）：
  - 第30-42秒：ReasmOK=0, ReasmFail=3000-3578/s → 100% 重组失败，雪崩
  - topic hz 降到 2.2 Hz，max 间隔 13-15 秒

修复后（ipfrag_high_thresh=64MB）：
  - 全程 ReasmOK=61-84/s，ReasmFail 仅零星 5-22/s
  - topic hz 稳定 8.0-9.8 Hz，max 间隔仅 0.5 秒
  - 50 秒全程无丢包
```

### 源码位置

| 文件 | 行号 | 说明 |
|------|------|------|
| `UDPv4Transport.cpp` | 431-447 | `get_binding_interfaces_list()` - 决定绑定地址 |
| `UDPv4Transport.cpp` | 287-317 | `OpenAndBindInputSocket()` - 绑定 socket |
| `UDPv4Transport.cpp` | 337-384 | `OpenInputChannel()` - 加入多播组 |
| `WriterHistory.cpp` | 320-321 | RTPS fragment size 计算（默认 65404 字节） |
| `RTPSWriter.cpp` | 299-322 | 最大数据大小计算 |
| `TransportInterface.h` | 33 | `s_maximumMessageSize = 65500` |

### 适用范围

所有接收大数据量 ROS2 topic 的机器都应配置此参数，包括：
- 用户电脑（接收雷达点云）
- Orin（如果也订阅大数据 topic）
- x86（如果也订阅大数据 topic）

判断标准：如果 topic 单帧大小 > MTU（1500字节），就会触发 IP 分片，就需要关注此参数。


> 关于 RTPS 分片、IP 分片、内核重组机制等网络知识的详细解释，参见**问题2：节点订阅数据的完整链路**。

---

## 问题8：什么是单播，什么时候会走单播

### 问题背景

在分析雷达数据丢包问题时，我们一直假设雷达数据通过多播（239.255.0.1）传输。但 tcpdump 抓包发现，雷达数据实际通过**单播**发送到 `192.168.0.100:13661`。这引出了一个关键问题：Fast-DDS 什么时候用多播，什么时候用单播？

### 问题回答

#### 1. 单播 vs 多播

| 特性 | 单播 (Unicast) | 多播 (Multicast) |
|------|---------------|-----------------|
| 目标地址 | 特定 IP（如 192.168.0.100） | 多播组 IP（如 239.255.0.1） |
| 接收者 | 一对一 | 一对多 |
| 可靠性 | 较高（点对点） | 较低（依赖组管理） |
| 适用场景 | 数据传输 | 发现协议 |
| 包大小 | 可达 65468 字节 | 通常 252 字节左右 |

#### 2. Fast-DDS 的两个阶段

**阶段一：发现阶段（metatraffic）→ 多播**

节点启动后，通过多播广播自己的存在，让其他节点发现自己：

```
节点 A 启动
    ↓
发送多播到 239.255.0.1:13650（话题发现）
发送多播到 224.0.0.251:13650（节点发现）
    ↓
所有同 domain 的节点都能收到
    ↓
节点之间交换 locator 信息（IP + 端口）
```

发现阶段交换的关键信息：
- 每个 participant 的**单播 metatraffic locator**（如 192.168.0.100:13660）
- 每个 participant 的**单播 user data locator**（如 192.168.0.100:13661）

**阶段二：数据传输阶段（user data）→ 单播**

发现完成后，发布者直接通过单播发送数据到订阅者：

```
Orin 发布雷达数据
    ↓
查找订阅者的 locator
    ↓
通过单播发送到 192.168.0.100:13661
    ↓
订阅者接收数据
```

#### 3. RTPS 端口分配规则

Fast-DDS 按以下公式分配端口（Domain ID = 25）：

| 用途 | 公式 | 端口号 | 传输方式 |
|------|------|--------|---------|
| 多播 metatraffic | PB + DG×domainId + d0 | **13650** | 多播 |
| 多播 user data | PB + DG×domainId + d2 | **13651** | 多播 |
| 单播 metatraffic | PB + DG×domainId + d1 + PG×participantId | **13660 + 2×N** | 单播 |
| 单播 user data | PB + DG×domainId + d3 + PG×participantId | **13661 + 2×N** | 单播 |

其中 PB=7400, DG=250, d0=0, d1=10, d2=1, d3=11, PG=2。

用户电脑上的实际端口分配：

```
Participant 0 (ROS2 daemon):
  metatraffic: 13660    user data: 13661

Participant 1 (其他进程):
  metatraffic: 13662    user data: 13663

Participant 2 (订阅者进程):
  metatraffic: 13664    user data: 13665
```

#### 4. 抓包验证

```bash
# 抓取大包（雷达数据）
sudo tcpdump -i enp4s0 -n -c 5 \
  'udp and src host 192.168.0.11 and dst host 192.168.0.100 and greater 1000'
```

结果：

```
192.168.0.11.45846 > 192.168.0.100.13661: UDP, length 65468
```

- 目标地址是 `192.168.0.100`（单播），不是 `239.255.0.1`（多播）
- 目标端口是 `13661`（participant 0 的 user data 端口）
- 之前在 13650 端口看到的 252 字节小包是**发现协议**的多播包，不是雷达数据

#### 5. 端口分布统计

```bash
sudo timeout 5 tcpdump -i enp4s0 -n \
  'udp and src host 192.168.0.11 and dst host 192.168.0.100' 2>&1 | \
  grep -oP '192\.168\.0\.100\.\d+' | sort | uniq -c | sort -rn
```

结果：

```
384 192.168.0.100.13661    ← participant 0 (daemon) 的 user data
136 192.168.0.100.13662    ← participant 1 的 metatraffic
136 192.168.0.100.13660    ← participant 0 (daemon) 的 metatraffic
  0 192.168.0.100.13665    ← participant 2 (订阅者) 的 user data ⚠️
```

#### 6. 核心发现：Orin 把数据发到了 daemon 的端口

**关键问题**：Orin 把雷达数据发到了 `192.168.0.100:13661`（daemon 的端口），而不是 `192.168.0.100:13665`（订阅者的端口）。

这意味着：
- Orin 在发现阶段获取到的订阅者 locator 指向了 **daemon（participant 0）**
- 订阅者（participant 2）的端口 13665 没有直接从 Orin 收到数据
- 订阅者能收到部分数据，说明 daemon 可能通过**进程间通讯**（共享内存或本地 socket）转发给订阅者

#### 7. 与应用层丢包的关系

这个发现解释了为什么应用层丢包严重：

```
Orin 发送雷达数据（单播）
    ↓
到达 192.168.0.100:13661（daemon 端口）
    ↓
daemon 接收数据
    ↓
daemon 通过进程间通讯转发给订阅者（13665）
    ↓ ← 这一步可能是瓶颈
订阅者接收数据
```

**可能的丢包原因**：
1. daemon 转发存在延迟或缓冲区限制
2. 进程间通讯的带宽不足以处理 0.5MB/帧 × 10Hz = 5MB/s 的数据
3. daemon 本身的处理能力成为瓶颈
4. 周期性断流可能与 daemon 的 GC 或内部缓冲区管理有关

**待验证**：
- daemon 是否真的在转发数据？
- 如果关闭 daemon（`ros2 daemon stop`），订阅者还能收到数据吗？
- Orin 为什么认为 daemon 是订阅者？是发现协议的问题还是 locator 配置的问题？

### 总结

| 阶段 | 传输方式 | 端口 | 包大小 |
|------|---------|------|--------|
| 节点发现 | 多播 → 239.255.0.1 | 13650 | ~252 字节 |
| 话题发现 | 多播 → 224.0.0.251 | 13650 | ~252 字节 |
| 数据传输 | 单播 → 192.168.0.100 | 13661 | ~65468 字节 |

应用层丢包的核心链路：**Orin → daemon(13661) → 订阅者(13665)**，中间经过 daemon 转发，这个转发环节可能是丢包的根本原因。

### 补充：为什么有些机器不用调 sysctl 就能正常接收？

#### 现象

同样的系统、同样的默认 sysctl 参数（`ipfrag_high_thresh=4MB`, `ipfrag_time=30s`），有些用户电脑不用任何调参就能稳定接收 10Hz 雷达数据，而某些电脑必须调大 `ipfrag_high_thresh` 才行。CPU 和内存都不是瓶颈。

#### 原因：网卡硬件和驱动差异

核心逻辑：**如果网卡层 fragment 丢失率极低，不完整的重组条目就不会在内核缓冲区堆积，默认 4MB 的 `ipfrag_high_thresh` 就够用。**

关键差异在于网卡的中断处理能力和驱动质量，而不是 Ring Buffer 大小本身。

#### 实际对比

| 项目 | 正常机器（不用调参） | 问题机器（必须调参） |
|------|---------------------|---------------------|
| 网卡 | Intel I219（`enp0s31f6`） | Realtek RTL8125（`enp4s0`） |
| 驱动 | `e1000e`（Intel 官方） | `r8169`（内核通用驱动） |
| RX Ring Buffer 最大值 | 4096 | 256 |
| RX Ring Buffer 当前值 | 256 | 256（已是上限） |
| 中断处理 | NAPI 机制成熟，DMA 效率高 | r8169 通用驱动对 RTL8125 优化不足 |
| 突发 fragment 处理 | 即使 RX=256 也能高效处理 | 突发流量下容易来不及收包 |

#### 为什么 Intel 网卡 RX=256 也不丢包

- Intel `e1000e` 驱动的 NAPI 中断合并机制成熟，DMA 调度高效
- 即使 Ring Buffer 只有 256，也能在突发 fragment 到达时及时处理
- fragment 在网卡层几乎不丢 → 内核重组缓冲区不堆积 → 默认 4MB 够用

#### 为什么 Realtek RTL8125 + r8169 会丢包

- `r8169` 是内核通用驱动，对 RTL8125 的中断处理和 DMA 调度不够优化
- Ring Buffer 上限只有 256，且无法调大
- 突发 fragment 到达时来不及处理 → 网卡层丢 fragment → 不完整重组条目堆积 → 撑爆 4MB → 雪崩

#### 查看方法

```bash
# 查看网卡型号
lspci | grep -i ethernet

# 查看驱动
ethtool -i enp4s0

# 查看 Ring Buffer（需要 ethtool）
ethtool -g enp4s0

# 无 ethtool 时查看网卡芯片 vendor/device ID
cat /sys/class/net/enp4s0/device/vendor /sys/class/net/enp4s0/device/device
# 0x10ec + 0x8125 = Realtek RTL8125
# 0x8086 + 0x15bc = Intel I219
```

#### 解决方案

1. **保持 sysctl 调参（推荐，最省事）**：已验证有效，持久化即可
   ```bash
   sudo tee /etc/sysctl.d/99-ipfrag.conf << 'EOF'
   net.ipv4.ipfrag_high_thresh = 67108864
   net.ipv4.ipfrag_low_thresh = 50331648
   net.ipv4.ipfrag_time = 5
   net.core.rmem_max = 67108864
   EOF
   ```

2. **换用 Realtek 官方 r8125 驱动**：官方驱动对 RTL8125 优化更好，Ring Buffer 上限可能更大，中断处理也更高效

3. **Intel 网卡可调大 Ring Buffer**：正常机器虽然不调也行，但可以进一步提升余量
   ```bash
   sudo ethtool -G enp0s31f6 rx 4096
   ```
---

## 总结

### 核心问题和解决方案

**问题1：为什么用户电脑需要设置 buffer**
- **原因**：系统默认缓冲区可能很小，Fast-DDS 自动调整只到 64KB
- **解决**：配置 receiveBufferSize 触发 set_option()，或增加 net.core.rmem_max

**问题2：节点订阅数据的完整链路**
- **链路**：网卡 → 内核缓冲区 → 应用层缓冲区 → 分片重组 → 反序列化 → 用户回调
- **关键**：内核缓冲区最关键，满了会导致内核级丢包

**问题3：为什么用户电脑频率不稳定**
- **原因**：多任务环境，CPU 负载高，缓冲区太小
- **unset 后更稳定**：启用共享内存，减轻网络负担
- **TCP 不适合**：延迟高，队头阻塞，违背实时性

**问题4：FASTDDS_BUILTIN_TRANSPORTS 的工作原理**
- **作用**：控制默认传输方式
- **LARGE_DATA**：启用 TCP，适合大数据但不适合实时数据
- **推荐**：雷达数据使用 DEFAULT 模式（UDP + SHM）

### 最终推荐配置

```bash
# 1. 增加系统缓冲区（所有机器）
sudo sysctl -w net.core.rmem_max=67108864   # 64MB
sudo sysctl -w net.core.wmem_max=67108864   # 64MB
sudo sysctl -w net.core.rmem_default=16777216  # 16MB

# 永久设置
echo "net.core.rmem_max=67108864" | sudo tee -a /etc/sysctl.conf
echo "net.core.wmem_max=67108864" | sudo tee -a /etc/sysctl.conf
echo "net.core.rmem_default=16777216" | sudo tee -a /etc/sysctl.conf
sudo sysctl -p

# 2. 使用默认配置（推荐）
unset FASTRTPS_DEFAULT_PROFILES_FILE
export FASTDDS_BUILTIN_TRANSPORTS=DEFAULT

# 3. 或者使用优化的 XML 配置
export FASTRTPS_DEFAULT_PROFILES_FILE=/path/to/optimized_config.xml
```

**优化的 XML 配置**：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<dds>
    <profiles xmlns="http://www.eprosima.com/XMLSchemas/fastRTPS_Profiles">
        <transport_descriptors>
            <transport_descriptor>
                <transport_id>UDPv4Transport</transport_id>
                <type>UDPv4</type>
                <interfaceWhiteList>
                    <address>192.168.0.100</address>
                </interfaceWhiteList>
                <sendBufferSize>67108864</sendBufferSize>      <!-- 64MB -->
                <receiveBufferSize>67108864</receiveBufferSize> <!-- 64MB -->
            </transport_descriptor>
            <transport_descriptor>
                <transport_id>SHMTransport</transport_id>
                <type>SHM</type>
            </transport_descriptor>
        </transport_descriptors>
        <participant profile_name="default_participant" is_default_profile="true">
            <rtps>
                <userTransports>
                    <transport_id>UDPv4Transport</transport_id>
                    <transport_id>SHMTransport</transport_id>
                </userTransports>
                <useBuiltinTransports>false</useBuiltinTransports>
            </rtps>
        </participant>
    </profiles>
</dds>
```

### 验证配置

```bash
# 1. 验证系统缓冲区
sysctl net.core.rmem_max
sysctl net.core.wmem_max

# 2. 启动 ROS2 节点后，查看实际的 socket 缓冲区
sudo ss -u -a -n -p -m | grep ros

# 3. 监控接收频率
ros2 topic hz /livox/lidar_front

# 4. 监控丢包统计
watch -n 1 'netstat -su | grep -i "receive buffer errors"'

# 5. 监控 CPU 负载
htop
```