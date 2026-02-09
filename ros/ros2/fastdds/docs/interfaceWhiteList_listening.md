# Fast-DDS interfaceWhiteList 限制监听接口

## 问题

如何让 PC A 只监听特定接口（如 192.168.0.11），而不监听另一个接口（如 10.11.12.131）？

## 答案

**是的，`interfaceWhiteList` 同时限制发送和接收（监听）接口！**

---

## interfaceWhiteList 的完整作用

`interfaceWhiteList` 不仅限制发送接口，也限制接收接口：

1. **发送端**：只在白名单接口上发送数据
2. **接收端**：只在白名单接口上绑定 socket 和监听

---

## 代码验证

### 1. 接收端绑定接口的代码

**文件：** `src/cpp/rtps/transport/UDPTransportInterface.cpp:213-241`

```cpp
bool UDPTransportInterface::OpenAndBindInputSockets(
        const Locator& locator,
        TransportReceiverInterface* receiver,
        bool is_multicast,
        uint32_t maxMsgSize)
{
    std::unique_lock<std::recursive_mutex> scopedLock(mInputMapMutex);

    try
    {
        // ★ 获取要绑定的接口列表（受白名单限制）
        std::vector<std::string> vInterfaces = get_binding_interfaces_list();

        // ★ 只在白名单接口上创建监听 socket
        for (std::string sInterface : vInterfaces)
        {
            UDPChannelResource* p_channel_resource;
            p_channel_resource = CreateInputChannelResource(sInterface, locator, is_multicast, maxMsgSize, receiver);
            mInputSockets[IPLocator::getPhysicalPort(locator)].push_back(p_channel_resource);
        }
    }
    catch (asio::system_error const& e)
    {
        logInfo(TRANSPORT_UDP, "UDPTransport Error binding at port: ("
                << IPLocator::getPhysicalPort(locator) << ")" << " with msg: " << e.what());
        mInputSockets.erase(IPLocator::getPhysicalPort(locator));
        return false;
    }

    return true;
}
```

### 2. get_binding_interfaces_list() 函数

**文件：** `src/cpp/rtps/transport/UDPv4Transport.cpp:446-462`

```cpp
std::vector<std::string> UDPv4Transport::get_binding_interfaces_list()
{
    std::vector<std::string> vOutputInterfaces;

    if (is_interface_whitelist_empty())
    {
        // 白名单为空：绑定所有接口（0.0.0.0）
        vOutputInterfaces.push_back(s_IPv4AddressAny);
    }
    else
    {
        // ★ 白名单不为空：只绑定白名单中的接口
        for (auto& ip : interface_whitelist_)
        {
            vOutputInterfaces.push_back(ip.to_string());
        }
    }

    return vOutputInterfaces;
}
```

---

## 工作原理

### 场景：PC A 有两个网卡

```
PC A:
- eth0: 192.168.0.11 (内网)
- eth1: 10.11.12.131 (外网)
```

### 不使用白名单（默认行为）

```
OpenAndBindInputSockets()
  ↓
get_binding_interfaces_list() 返回 ["0.0.0.0"]
  ↓
绑定到 0.0.0.0:7400
  ↓
监听所有接口（eth0 和 eth1）
  ↓
可以从任何接口接收消息
```

### 使用白名单（只监听 192.168.0.11）

**XML 配置：**
```xml
<?xml version="1.0" encoding="UTF-8" ?>
<dds>
    <profiles>
        <transport_descriptors>
            <transport_descriptor>
                <transport_id>CustomUDPv4</transport_id>
                <type>UDPv4</type>
                <interfaceWhiteList>
                    <address>192.168.0.11</address>
                </interfaceWhiteList>
            </transport_descriptor>
        </transport_descriptors>

        <participant profile_name="participant_profile" is_default_profile="true">
            <rtps>
                <userTransports>
                    <transport_id>CustomUDPv4</transport_id>
                </userTransports>
                <useBuiltinTransports>false</useBuiltinTransports>
            </rtps>
        </participant>
    </profiles>
</dds>
```

**执行流程：**
```
OpenAndBindInputSockets()
  ↓
get_binding_interfaces_list() 返回 ["192.168.0.11"]
  ↓
绑定到 192.168.0.11:7400
  ↓
只监听 eth0 接口
  ↓
只能从 192.168.0.11 接收消息
  ↓
从 10.11.12.131 发来的消息会被忽略 ✅
```

---

## 实际效果对比

### 场景 1：不使用白名单

| 发送方 | 发送接口 | PC A 能否收到 |
|--------|---------|--------------|
| PC B (192.168.0.10) | 192.168.0.x 网段 | ✅ 能收到 |
| PC C (10.11.12.100) | 10.11.12.x 网段 | ✅ 能收到 |

### 场景 2：使用白名单（只监听 192.168.0.11）

| 发送方 | 发送接口 | PC A 能否收到 |
|--------|---------|--------------|
| PC B (192.168.0.10) | 192.168.0.x 网段 | ✅ 能收到 |
| PC C (10.11.12.100) | 10.11.12.x 网段 | ❌ **不能收到** |

---

## 完整配置示例

### 方法 1：使用环境变量 + XML 文件

**1. 创建 XML 配置文件 `fastdds_whitelist.xml`：**

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<dds>
    <profiles>
        <transport_descriptors>
            <transport_descriptor>
                <transport_id>CustomUDPv4</transport_id>
                <type>UDPv4</type>
                <interfaceWhiteList>
                    <address>192.168.0.11</address>
                </interfaceWhiteList>
            </transport_descriptor>
        </transport_descriptors>

        <participant profile_name="participant_profile" is_default_profile="true">
            <rtps>
                <userTransports>
                    <transport_id>CustomUDPv4</transport_id>
                </userTransports>
                <useBuiltinTransports>false</useBuiltinTransports>
            </rtps>
        </participant>
    </profiles>
</dds>
```

**2. 设置环境变量并启动节点：**

```bash
export FASTRTPS_DEFAULT_PROFILES_FILE=/path/to/fastdds_whitelist.xml
ros2 run demo_nodes_cpp talker
```

---

### 方法 2：使用代码配置（C++）

```cpp
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

int main()
{
    // 创建 Participant QoS
    DomainParticipantQos pqos;

    // 禁用内置传输
    pqos.transport().use_builtin_transports = false;

    // 创建自定义 UDP 传输
    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();

    // ★ 设置白名单：只监听 192.168.0.11
    udp_transport->interfaceWhiteList.push_back("192.168.0.11");

    // 添加传输到 QoS
    pqos.transport().user_transports.push_back(udp_transport);

    // 创建 Participant
    DomainParticipant* participant =
        DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    // ... 创建 Publisher/Subscriber ...

    return 0;
}
```

---

### 方法 3：使用代码配置（Python - ROS 2）

```python
import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile

class WhitelistNode(Node):
    def __init__(self):
        super().__init__('whitelist_node')

        # 注意：Python API 可能不直接支持 transport 配置
        # 推荐使用 XML 配置文件 + 环境变量的方式

        self.publisher_ = self.create_publisher(
            String,
            'topic',
            10
        )

def main():
    rclpy.init()
    node = WhitelistNode()
    rclpy.spin(node)
    rclpy.shutdown()
```

**对于 Python，推荐使用 XML + 环境变量方式：**

```bash
export FASTRTPS_DEFAULT_PROFILES_FILE=/path/to/fastdds_whitelist.xml
python3 your_node.py
```

---

## 验证方法

### 1. 使用 netstat 查看绑定接口

```bash
# 启动节点后，查看绑定的端口
sudo netstat -tulnp | grep 7400

# 不使用白名单的输出：
# udp  0.0.0.0:7400  0.0.0.0:*  12345/talker

# 使用白名单的输出：
# udp  192.168.0.11:7400  0.0.0.0:*  12345/talker
```

### 2. 使用 tcpdump 抓包验证

**在 PC A 上启动抓包：**

```bash
# 监听 eth0 (192.168.0.11)
sudo tcpdump -i eth0 -n 'udp and dst port 7400' -v

# 监听 eth1 (10.11.12.131)
sudo tcpdump -i eth1 -n 'udp and dst port 7400' -v
```

**测试：**
- 从 192.168.0.x 网段发送消息 → eth0 应该能抓到包
- 从 10.11.12.x 网段发送消息 → eth1 不应该抓到包（如果使用了白名单）

### 3. 使用 Fast-DDS 日志

**启用详细日志：**

```bash
export FASTDDS_ENVIRONMENT_FILE=/path/to/log_config.xml
export FASTRTPS_DEFAULT_PROFILES_FILE=/path/to/fastdds_whitelist.xml
ros2 run demo_nodes_cpp talker
```

**log_config.xml：**
```xml
<?xml version="1.0" encoding="UTF-8" ?>
<dds>
    <log>
        <use_default>true</use_default>
        <consumer>
            <class>StdoutConsumer</class>
        </consumer>
    </log>
</dds>
```

查看日志中的 "Binding" 或 "Opening input channel" 相关信息。

---

## 多接口白名单

如果你想监听多个接口，可以在白名单中添加多个地址：

```xml
<interfaceWhiteList>
    <address>192.168.0.11</address>
    <address>192.168.1.20</address>
</interfaceWhiteList>
```

这样会在两个接口上都创建监听 socket。

---

## 注意事项

### 1. 必须禁用内置传输

使用自定义传输配置时，必须设置：

```xml
<useBuiltinTransports>false</useBuiltinTransports>
```

否则 Fast-DDS 会同时使用内置传输和自定义传输，白名单可能不会完全生效。

### 2. 接口地址必须存在

白名单中的 IP 地址必须是本机实际存在的网络接口地址，否则会报错：

```cpp
// UDPv4Transport.cpp:119-124
if (interface_whitelist_.empty())
{
    logError(TRANSPORT, "All whitelist interfaces were filtered out");
    interface_whitelist_.emplace_back(ip::address_v4::from_string("192.0.2.0"));
}
```

### 3. 组播需要特别注意

对于组播通信，白名单会影响：
- 绑定的接口
- 加入组播组的接口

**代码：** `src/cpp/rtps/transport/UDPv4Transport.cpp:386-389`

```cpp
// Join group on all whitelisted interfaces
for (auto& ip : interface_whitelist_)
{
    p_channel_resource->socket()->set_option(ip::multicast::join_group(locatorAddress, ip));
}
```

### 4. 进程级别配置

记住：配置是**进程级别**的，不是全局的。每个进程需要单独配置。

---

## 总结

| 特性 | 说明 |
|------|------|
| **作用范围** | 同时限制发送和接收接口 |
| **配置方式** | XML 文件 + 环境变量 或 代码配置 |
| **效果** | 只在白名单接口上绑定 socket 和监听 |
| **多接口** | 支持配置多个接口 |
| **验证方法** | netstat、tcpdump、Fast-DDS 日志 |

**关键理解：**
- `interfaceWhiteList` 控制 `get_binding_interfaces_list()` 的返回值
- 接收端调用 `OpenAndBindInputSockets()` 时只在白名单接口上创建 socket
- 这样就实现了**只监听特定接口**的效果

---

**文档版本：** v2.6.x
**生成日期：** 2026-02-09
**代码库：** `/home/astribot/workspace/ros2/src/eProsima/Fast-DDS`
