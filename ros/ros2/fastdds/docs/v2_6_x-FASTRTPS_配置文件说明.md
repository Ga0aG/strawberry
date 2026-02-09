# Fast-DDS XML 配置文件完整说明

## 概述

**FASTRTPS_DEFAULT_PROFILES_FILE** 是 Fast-DDS (原 Fast-RTPS) 的默认配置文件，用于配置 DDS 应用程序的各种参数。

### 文件位置

1. **默认文件名**: `DEFAULT_FASTRTPS_PROFILES.xml`
2. **环境变量**: `FASTRTPS_DEFAULT_PROFILES_FILE` - 可以指定自定义路径
3. **跳过加载**: 设置环境变量 `SKIP_DEFAULT_XML_FILE=1` 可跳过默认配置文件加载

### 加载顺序

```
1. 检查环境变量 SKIP_DEFAULT_XML_FILE
2. 如果未设置，查找当前目录下的 DEFAULT_FASTRTPS_PROFILES.xml
3. 或者加载 FASTRTPS_DEFAULT_PROFILES_FILE 环境变量指定的文件
```

---

## XML 层级结构

```
<dds>                                          # 根元素
├── <profiles>                                 # 配置文件集合（主要部分）
│   ├── <library_settings>                    # 库全局设置
│   ├── <transport_descriptors>               # 传输层描述符
│   │   └── <transport_descriptor> (多个)     # 单个传输配置
│   ├── <participant> (多个)                  # DomainParticipant 配置
│   │   ├── domainId                          # 域 ID
│   │   └── <rtps>                            # RTPS 协议配置
│   │       ├── <allocation>                  # 资源分配
│   │       ├── <builtin>                     # 内置端点
│   │       │   ├── <discovery_config>        # 发现配置
│   │       │   ├── <metatrafficUnicastLocatorList>
│   │       │   ├── <metatrafficMulticastLocatorList>
│   │       │   └── <initialPeersList>        # 初始对等节点
│   │       ├── <port>                        # 端口配置
│   │       ├── <userTransports>              # 用户传输
│   │       └── <propertiesPolicy>            # 属性策略
│   ├── <publisher> (多个)                    # Publisher/DataWriter 配置
│   │   ├── <topic>                           # 主题配置
│   │   │   ├── <historyQos>                  # 历史 QoS
│   │   │   └── <resourceLimitsQos>           # 资源限制
│   │   ├── <qos>                             # QoS 策略
│   │   │   ├── <durability>                  # 持久性
│   │   │   ├── <reliability>                 # 可靠性
│   │   │   ├── <liveliness>                  # 活跃性
│   │   │   ├── <partition>                   # 分区
│   │   │   └── ...                           # 其他 QoS
│   │   ├── <times>                           # Writer 时间配置
│   │   ├── <unicastLocatorList>              # 单播定位器
│   │   └── <multicastLocatorList>            # 组播定位器
│   ├── <subscriber> (多个)                   # Subscriber/DataReader 配置
│   │   ├── <topic>                           # 主题配置
│   │   ├── <qos>                             # QoS 策略
│   │   ├── <times>                           # Reader 时间配置
│   │   └── ...                               # 定位器等
│   ├── <requester> (多个)                    # RPC 请求者配置
│   └── <replier> (多个)                      # RPC 应答者配置
├── <types>                                    # 动态类型定义
│   ├── <struct>                              # 结构体
│   ├── <enum>                                # 枚举
│   ├── <union>                               # 联合体
│   └── ...                                   # 其他类型
└── <log>                                      # 日志配置
    └── <consumer>                            # 日志消费者
```

---

## 主要配置项详解

### 1. Library Settings（库全局设置）

```xml
<library_settings>
    <intraprocess_delivery>FULL</intraprocess_delivery>
</library_settings>
```

**intraprocess_delivery** 选项：
- `OFF`: 禁用进程内通信
- `USER_DATA_ONLY`: 仅用户数据使用进程内通信
- `FULL`: 完全使用进程内通信（默认推荐）

---

### 2. Transport Descriptors（传输层描述符）

#### UDPv4 传输

```xml
<transport_descriptor>
    <transport_id>udp_transport</transport_id>
    <type>UDPv4</type>
    <sendBufferSize>65536</sendBufferSize>        <!-- 发送缓冲区大小 -->
    <receiveBufferSize>65536</receiveBufferSize>  <!-- 接收缓冲区大小 -->
    <TTL>1</TTL>                                   <!-- 生存时间 -->
    <non_blocking_send>false</non_blocking_send>  <!-- 非阻塞发送 -->
    <maxMessageSize>65500</maxMessageSize>         <!-- 最大消息大小 -->
    <maxInitialPeersRange>4</maxInitialPeersRange><!-- 最大初始对等节点范围 -->
    <interfaceWhiteList>                           <!-- 接口白名单 -->
        <address>127.0.0.1</address>
        <address>192.168.1.0</address>
    </interfaceWhiteList>
</transport_descriptor>
```

**默认值**：
- sendBufferSize: 0 (系统默认)
- receiveBufferSize: 0 (系统默认)
- TTL: 1
- maxMessageSize: 65500 字节

#### TCPv4 传输

```xml
<transport_descriptor>
    <transport_id>tcp_transport</transport_id>
    <type>TCPv4</type>
    <keep_alive_frequency_ms>5000</keep_alive_frequency_ms>
    <keep_alive_timeout_ms>15000</keep_alive_timeout_ms>
    <max_logical_port>65535</max_logical_port>
    <logical_port_range>20</logical_port_range>
    <listening_ports>
        <port>5100</port>
    </listening_ports>
    <enable_tcp_nodelay>true</enable_tcp_nodelay>
</transport_descriptor>
```

#### 共享内存传输（SHM）

```xml
<transport_descriptor>
    <transport_id>shm_transport</transport_id>
    <type>SHM</type>
    <segment_size>524288</segment_size>              <!-- 段大小 -->
    <port_queue_capacity>512</port_queue_capacity>   <!-- 端口队列容量 -->
    <healthy_check_timeout_ms>1000</healthy_check_timeout_ms>
</transport_descriptor>
```

---

### 3. Participant（参与者配置）

#### 基本配置

```xml
<participant profile_name="default_participant_profile" is_default_profile="true">
    <domainId>0</domainId>  <!-- 域 ID，默认 0 -->
    <rtps>
        <!-- RTPS 配置 -->
    </rtps>
</participant>
```

**is_default_profile** 属性：
- `true`: 设置为默认配置文件
- `false`: 非默认配置文件

#### 资源分配（Allocation）

```xml
<allocation>
    <remote_locators>
        <max_unicast_locators>4</max_unicast_locators>
        <max_multicast_locators>1</max_multicast_locators>
    </remote_locators>
    <total_participants>
        <initial>0</initial>    <!-- 初始分配数量 -->
        <maximum>0</maximum>    <!-- 最大数量，0 表示无限制 -->
        <increment>1</increment><!-- 增量 -->
    </total_participants>
    <total_readers>
        <initial>0</initial>
        <maximum>0</maximum>
        <increment>1</increment>
    </total_readers>
    <total_writers>
        <initial>0</initial>
        <maximum>0</maximum>
        <increment>1</increment>
    </total_writers>
</allocation>
```

**默认值**：
- initial: 0
- maximum: 0 (无限制)
- increment: 1

#### 发现配置（Discovery）

```xml
<discovery_config>
    <discoveryProtocol>SIMPLE</discoveryProtocol>
    <EDP>SIMPLE</EDP>
    <ignoreParticipantFlags>FILTER_DIFFERENT_PROCESS</ignoreParticipantFlags>

    <leaseAnnouncement>
        <sec>3</sec>
        <nanosec>0</nanosec>
    </leaseAnnouncement>

    <leaseDuration>
        <sec>20</sec>
        <nanosec>0</nanosec>
    </leaseDuration>

    <initialAnnouncements>
        <count>5</count>
        <period>
            <sec>0</sec>
            <nanosec>100000000</nanosec>
        </period>
    </initialAnnouncements>
</discovery_config>
```

**discoveryProtocol** 选项：
- `SIMPLE`: 简单发现协议（默认）
- `CLIENT`: 客户端模式
- `SERVER`: 服务器模式
- `BACKUP`: 备份服务器
- `SUPER_CLIENT`: 超级客户端
- `NONE`: 禁用发现

**EDP** 选项：
- `SIMPLE`: 简单端点发现协议
- `STATIC`: 静态端点发现

**ignoreParticipantFlags** 选项：
- `FILTER_DIFFERENT_HOST`: 过滤不同主机
- `FILTER_DIFFERENT_PROCESS`: 过滤不同进程
- `FILTER_SAME_PROCESS`: 过滤相同进程

**默认值**：
- leaseAnnouncement: 3 秒
- leaseDuration: 20 秒
- initialAnnouncements count: 5 次

#### 端口配置（Port）

```xml
<port>
    <portBase>7400</portBase>
    <domainIDGain>250</domainIDGain>
    <participantIDGain>2</participantIDGain>
    <offsetd0>0</offsetd0>
    <offsetd1>10</offsetd1>
    <offsetd2>1</offsetd2>
    <offsetd3>11</offsetd3>
</port>
```

**端口计算公式**：
```
端口 = portBase + domainIDGain * domainId + participantIDGain * participantId + offset
```

**默认值**：
- portBase: 7400
- domainIDGain: 250
- participantIDGain: 2

---

### 4. Publisher/DataWriter（发布者配置）

#### 主题配置（Topic）

```xml
<topic>
    <kind>NO_KEY</kind>        <!-- NO_KEY | WITH_KEY -->
    <name>DefaultTopic</name>
    <dataType>DefaultType</dataType>

    <historyQos>
        <kind>KEEP_LAST</kind> <!-- KEEP_LAST | KEEP_ALL -->
        <depth>1</depth>
    </historyQos>

    <resourceLimitsQos>
        <max_samples>5000</max_samples>
        <max_instances>10</max_instances>
        <max_samples_per_instance>400</max_samples_per_instance>
        <allocated_samples>100</allocated_samples>
    </resourceLimitsQos>
</topic>
```

**kind** 选项：
- `NO_KEY`: 无键主题
- `WITH_KEY`: 有键主题

**historyQos kind** 选项：
- `KEEP_LAST`: 保留最后 N 个样本
- `KEEP_ALL`: 保留所有样本

**默认值**：
- kind: NO_KEY
- historyQos kind: KEEP_LAST
- depth: 1
- max_samples: 5000

#### QoS 策略

##### 持久性（Durability）

```xml
<durability>
    <kind>VOLATILE</kind>
</durability>
```

**kind** 选项：
- `VOLATILE`: 易失性（默认）- 不保存历史数据
- `TRANSIENT_LOCAL`: 瞬态本地 - 保存本地历史数据
- `TRANSIENT`: 瞬态 - 保存历史数据
- `PERSISTENT`: 持久化 - 永久保存数据

##### 可靠性（Reliability）

```xml
<reliability>
    <kind>RELIABLE</kind>
    <max_blocking_time>
        <sec>0</sec>
        <nanosec>100000000</nanosec>
    </max_blocking_time>
</reliability>
```

**kind** 选项：
- `BEST_EFFORT`: 尽力而为（默认 Subscriber）
- `RELIABLE`: 可靠传输（默认 Publisher）

**默认值**：
- max_blocking_time: 100 毫秒

##### 活跃性（Liveliness）

```xml
<liveliness>
    <kind>AUTOMATIC</kind>
    <lease_duration>
        <sec>2147483647</sec>
        <nanosec>4294967295</nanosec>
    </lease_duration>
    <announcement_period>
        <sec>2147483647</sec>
        <nanosec>4294967295</nanosec>
    </announcement_period>
</liveliness>
```

**kind** 选项：
- `AUTOMATIC`: 自动（默认）
- `MANUAL_BY_PARTICIPANT`: 手动按参与者
- `MANUAL_BY_TOPIC`: 手动按主题

**默认值**：
- lease_duration: 无限
- announcement_period: 无限

##### 所有权（Ownership）

```xml
<ownership>
    <kind>SHARED</kind>
</ownership>

<ownershipStrength>
    <value>0</value>
</ownershipStrength>
```

**kind** 选项：
- `SHARED`: 共享（默认）
- `EXCLUSIVE`: 独占

##### 分区（Partition）

```xml
<partition>
    <names>
        <name>partition_1</name>
        <name>partition_2</name>
    </names>
</partition>
```

**默认值**：空（无分区）

##### 发布模式（Publish Mode）

```xml
<publishMode>
    <kind>SYNCHRONOUS</kind>
    <flow_controller_name>custom_flow_controller</flow_controller_name>
</publishMode>
```

**kind** 选项：
- `SYNCHRONOUS`: 同步（默认）
- `ASYNCHRONOUS`: 异步

#### Writer 时间配置

```xml
<times>
    <initialHeartbeatDelay>
        <sec>0</sec>
        <nanosec>12000000</nanosec>
    </initialHeartbeatDelay>
    <heartbeatPeriod>
        <sec>3</sec>
        <nanosec>0</nanosec>
    </heartbeatPeriod>
    <nackResponseDelay>
        <sec>0</sec>
        <nanosec>5000000</nanosec>
    </nackResponseDelay>
    <nackSupressionDuration>
        <sec>0</sec>
        <nanosec>0</nanosec>
    </nackSupressionDuration>
</times>
```

**默认值**：
- initialHeartbeatDelay: 12 毫秒
- heartbeatPeriod: 3 秒
- nackResponseDelay: 5 毫秒
- nackSupressionDuration: 0

#### 历史内存策略

```xml
<historyMemoryPolicy>PREALLOCATED</historyMemoryPolicy>
```

**选项**：
- `PREALLOCATED`: 预分配（默认）
- `PREALLOCATED_WITH_REALLOC`: 预分配可重新分配
- `DYNAMIC`: 动态分配
- `DYNAMIC_REUSABLE`: 动态可重用

---

### 5. Subscriber/DataReader（订阅者配置）

订阅者配置与发布者类似，主要区别：

#### Reader 时间配置

```xml
<times>
    <initialAcknackDelay>
        <sec>0</sec>
        <nanosec>70000000</nanosec>
    </initialAcknackDelay>
    <heartbeatResponseDelay>
        <sec>0</sec>
        <nanosec>5000000</nanosec>
    </heartbeatResponseDelay>
</times>
```

**默认值**：
- initialAcknackDelay: 70 毫秒
- heartbeatResponseDelay: 5 毫秒

#### 期望内联 QoS

```xml
<expectsInlineQos>false</expectsInlineQos>
```

**默认值**：false

---

### 6. 定位器（Locators）

#### UDPv4 定位器

```xml
<locator>
    <udpv4>
        <port>7400</port>
        <address>192.168.1.100</address>
    </udpv4>
</locator>
```

#### UDPv6 定位器

```xml
<locator>
    <udpv6>
        <port>7400</port>
        <address>::1</address>
    </udpv6>
</locator>
```

#### TCPv4 定位器

```xml
<locator>
    <tcpv4>
        <port>5100</port>
        <physical_port>5100</physical_port>
        <address>192.168.1.100</address>
        <wan_address>203.0.113.1</wan_address>
        <unique_lan_id>192.168.001.001.05.100</unique_lan_id>
    </tcpv4>
</locator>
```

---

### 7. 动态类型（Types）

#### 结构体

```xml
<struct name="HelloWorld">
    <member name="index" type="uint32"/>
    <member name="message" type="string"/>
</struct>
```

#### 枚举

```xml
<enum name="Color">
    <enumerator name="RED" value="0"/>
    <enumerator name="GREEN" value="1"/>
    <enumerator name="BLUE" value="2"/>
</enum>
```

#### 联合体

```xml
<union name="MyUnion">
    <discriminator type="long"/>
    <case>
        <caseDiscriminator value="0"/>
        <member name="first" type="long"/>
    </case>
    <case>
        <caseDiscriminator value="1"/>
        <member name="second" type="string"/>
    </case>
</union>
```

---

### 8. 日志配置（Log）

```xml
<log>
    <use_default>true</use_default>
    <consumer>
        <class>StdoutConsumer</class>
        <property>
            <name>stderr_threshold</name>
            <value>Log::Kind::Error</value>
        </property>
    </consumer>
</log>
```

**consumer class** 选项：
- `StdoutConsumer`: 标准输出
- `StdoutErrConsumer`: 标准错误输出
- `FileConsumer`: 文件输出

---

## 常用配置场景

### 场景 1：本地测试（单机多进程）

```xml
<participant profile_name="local_test" is_default_profile="true">
    <domainId>0</domainId>
    <rtps>
        <useBuiltinTransports>true</useBuiltinTransports>
        <builtin>
            <discovery_config>
                <discoveryProtocol>SIMPLE</discoveryProtocol>
                <ignoreParticipantFlags>FILTER_DIFFERENT_PROCESS</ignoreParticipantFlags>
            </discovery_config>
        </builtin>
    </rtps>
</participant>
```

### 场景 2：跨网络通信（TCP）

```xml
<transport_descriptors>
    <transport_descriptor>
        <transport_id>tcp_transport</transport_id>
        <type>TCPv4</type>
        <listening_ports>
            <port>5100</port>
        </listening_ports>
    </transport_descriptor>
</transport_descriptors>

<participant profile_name="tcp_client">
    <rtps>
        <userTransports>
            <id>tcp_transport</id>
        </userTransports>
        <useBuiltinTransports>false</useBuiltinTransports>
        <builtin>
            <initialPeersList>
                <locator>
                    <tcpv4>
                        <address>192.168.1.100</address>
                        <port>5100</port>
                    </tcpv4>
                </locator>
            </initialPeersList>
        </builtin>
    </rtps>
</participant>
```

### 场景 3：高性能共享内存

```xml
<library_settings>
    <intraprocess_delivery>FULL</intraprocess_delivery>
</library_settings>

<transport_descriptors>
    <transport_descriptor>
        <transport_id>shm_transport</transport_id>
        <type>SHM</type>
        <segment_size>2097152</segment_size>
    </transport_descriptor>
</transport_descriptors>

<participant profile_name="shm_participant">
    <rtps>
        <userTransports>
            <id>shm_transport</id>
        </userTransports>
        <useBuiltinTransports>false</useBuiltinTransports>
    </rtps>
</participant>
```

### 场景 4：可靠通信 + 持久化

```xml
<publisher profile_name="reliable_persistent">
    <qos>
        <durability>
            <kind>TRANSIENT_LOCAL</kind>
        </durability>
        <reliability>
            <kind>RELIABLE</kind>
        </reliability>
    </qos>
    <topic>
        <historyQos>
            <kind>KEEP_ALL</kind>
        </historyQos>
    </topic>
</publisher>
```

---

## 总结

### 关键默认值速查表

| 配置项 | 默认值 | 说明 |
|--------|--------|------|
| domainId | 0 | 域 ID |
| discoveryProtocol | SIMPLE | 发现协议 |
| leaseAnnouncement | 3 秒 | 租约公告周期 |
| leaseDuration | 20 秒 | 租约持续时间 |
| historyQos kind | KEEP_LAST | 历史类型 |
| historyQos depth | 1 | 历史深度 |
| durability | VOLATILE | 持久性 |
| reliability (Publisher) | RELIABLE | 可靠性 |
| reliability (Subscriber) | BEST_EFFORT | 可靠性 |
| ownership | SHARED | 所有权 |
| historyMemoryPolicy | PREALLOCATED | 内存策略 |
| portBase | 7400 | 端口基数 |

### 配置文件优先级

1. 代码中显式设置的参数（最高优先级）
2. XML 配置文件中的参数
3. 系统默认值（最低优先级）

### 最佳实践

1. **使用 profile_name 和 is_default_profile**：为不同场景创建多个配置文件
2. **合理设置资源限制**：避免内存浪费
3. **选择合适的传输层**：本地用 SHM，跨网络用 TCP/UDP
4. **配置发现协议**：大规模部署考虑使用 Discovery Server
5. **调整 QoS 策略**：根据应用需求选择可靠性和持久性
6. **监控和日志**：配置适当的日志级别

---

## 参考资源

- [Fast-DDS 官方文档](https://fast-dds.docs.eprosima.com/)
- [XML 配置文件 XSD 定义](resources/xsd/fastRTPS_profiles.xsd)
- 示例配置文件：`FASTRTPS_DEFAULT_PROFILES_EXAMPLE.xml`
