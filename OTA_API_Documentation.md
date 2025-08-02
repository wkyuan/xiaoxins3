# OTA (Over-The-Air) API 文档

## 概述

本文档描述了基于 ESP32 的 OTA 更新系统的 API 接口，包括版本检查、配置更新和设备激活功能。

## 1. 版本检查接口

### 1.1 请求参数

#### HTTP 头部参数

| 参数名 | 类型 | 描述 | 示例值 |
|--------|------|------|--------|
| `Activation-Version` | String | 激活版本号，"2"表示有序列号，"1"表示无序列号 | "2" 或 "1" |
| `Device-Id` | String | 设备 MAC 地址 | "AA:BB:CC:DD:EE:FF" |
| `Client-Id` | String | 设备唯一标识符 (UUID) | "550e8400-e29b-41d4-a716-446655440000" |
| `Serial-Number` | String | 设备序列号 (仅当 Activation-Version="2" 时存在) | "ABC123456789" |
| `User-Agent` | String | 设备型号和固件版本 | "BOARD_NAME/1.0.0" |
| `Accept-Language` | String | 语言代码 | "zh-CN" |
| `Content-Type` | String | 内容类型 | "application/json" |

#### 请求方法
- **GET**: 当没有额外数据需要发送时
- **POST**: 当需要发送设备信息 JSON 数据时

#### 请求体 (POST 时)
```json
{
  "version": 2,
  "language": "zh-CN",
  "flash_size": 16777216,
  "minimum_free_heap_size": 8296340,
  "mac_address": "30:ed:a0:20:89:0c",
  "uuid": "cafe261c-414d-4c1a-b08d-e178c414f18c",
  "chip_model_name": "esp32s3",
  "chip_info": {
    "model": 9,
    "cores": 2,
    "revision": 2,
    "features": 18
  },
  "application": {
    "name": "xiaoxin",
    "version": "1.5.7",
    "compile_time": "May 23 2025T17:05:46Z",
    "idf_version": "v5.4.1-dirty",
    "elf_sha256": "8c2c24b468d28f132f24d26787a59a18c7a3dc45efae0b5cc2d2fb8be23cd927"
  },
  "partition_table": [
    {
      "label": "nvs",
      "type": 1,
      "subtype": 2,
      "address": 36864,
      "size": 16384
    },
    {
      "label": "otadata",
      "type": 1,
      "subtype": 0,
      "address": 53248,
      "size": 8192
    },
    {
      "label": "phy_init",
      "type": 1,
      "subtype": 1,
      "address": 61440,
      "size": 4096
    },
    {
      "label": "model",
      "type": 1,
      "subtype": 130,
      "address": 65536,
      "size": 983040
    },
    {
      "label": "ota_0",
      "type": 0,
      "subtype": 16,
      "address": 1048576,
      "size": 6291456
    },
    {
      "label": "ota_1",
      "type": 0,
      "subtype": 17,
      "address": 7340032,
      "size": 6291456
    }
  ],
  "ota": {
    "label": "ota_0"
  },
  "board": {
    "type": "xiaoxin-esp32s3-028",
    "name": "xiaoxin-esp32s3-028",
    "ssid": "CMCC-6T6Q",
    "rssi": -48,
    "channel": 6,
    "ip": "192.168.10.99",
    "mac": "30:ed:a0:20:89:0c",
    "device_token": ""
  }
}
```

#### 请求体字段说明

| 字段 | 类型 | 描述 |
|------|------|------|
| `version` | Number | API 版本号 |
| `language` | String | 设备语言设置 |
| `flash_size` | Number | Flash 存储大小（字节） |
| `minimum_free_heap_size` | Number | 最小可用堆内存大小（字节） |
| `mac_address` | String | 设备 MAC 地址 |
| `uuid` | String | 设备唯一标识符 |
| `chip_model_name` | String | 芯片型号名称 |

##### chip_info 对象
| 字段 | 类型 | 描述 |
|------|------|------|
| `model` | Number | 芯片型号代码 |
| `cores` | Number | CPU 核心数 |
| `revision` | Number | 芯片版本号 |
| `features` | Number | 芯片特性位掩码 |

##### application 对象
| 字段 | 类型 | 描述 |
|------|------|------|
| `name` | String | 应用程序名称 |
| `version` | String | 应用程序版本 |
| `compile_time` | String | 编译时间 |
| `idf_version` | String | ESP-IDF 版本 |
| `elf_sha256` | String | ELF 文件 SHA256 哈希 |

##### partition_table 数组
每个分区对象包含：
| 字段 | 类型 | 描述 |
|------|------|------|
| `label` | String | 分区标签名 |
| `type` | Number | 分区类型 (0=app, 1=data) |
| `subtype` | Number | 分区子类型 |
| `address` | Number | 分区起始地址 |
| `size` | Number | 分区大小（字节） |

**常见分区类型说明：**
- `nvs`: 非易失性存储 (NVS)
- `otadata`: OTA 数据分区
- `phy_init`: PHY 初始化数据
- `model`: 模型数据分区
- `ota_0/ota_1`: OTA 应用分区

##### ota 对象
| 字段 | 类型 | 描述 |
|------|------|------|
| `label` | String | 当前运行的 OTA 分区标签 |

##### board 对象
| 字段 | 类型 | 描述 |
|------|------|------|
| `type` | String | 开发板类型 |
| `name` | String | 开发板名称 |
| `ssid` | String | 连接的 WiFi SSID |
| `rssi` | Number | WiFi 信号强度 (dBm) |
| `channel` | Number | WiFi 信道 |
| `ip` | String | 设备 IP 地址 |
| `mac` | String | 网络接口 MAC 地址 |
| `device_token` | String | 设备令牌（可选） |

### 1.2 响应格式

```json
{
  "activation": {
    "message": "激活消息",
    "code": "激活码",
    "challenge": "HMAC 挑战字符串",
    "timeout_ms": 30000
  },
  "mqtt": {
    "server": "mqtt.example.com",
    "port": 1883,
    "username": "device_user",
    "password": "device_pass"
  },
  "websocket": {
    "url": "wss://ws.example.com",
    "port": 443
  },
  "server_time": {
    "timestamp": 1640995200000,
    "timezone_offset": 480
  },
  "firmware": {
    "version": "1.1.0",
    "url": "https://example.com/firmware.bin",
    "force": 0
  }
}
```

### 1.3 响应字段说明

#### activation 对象
| 字段 | 类型 | 必需 | 描述 |
|------|------|------|------|
| `message` | String | 否 | 激活相关的提示消息 |
| `code` | String | 否 | 激活码 |
| `challenge` | String | 否 | 用于 HMAC 计算的挑战字符串 |
| `timeout_ms` | Number | 否 | 激活超时时间（毫秒） |

#### mqtt 对象
动态配置对象，包含 MQTT 连接参数：
- 支持字符串和数字类型的配置项
- 自动更新本地 MQTT 设置

#### websocket 对象
动态配置对象，包含 WebSocket 连接参数：
- 支持字符串和数字类型的配置项
- 自动更新本地 WebSocket 设置

#### server_time 对象
| 字段 | 类型 | 必需 | 描述 |
|------|------|------|------|
| `timestamp` | Number | 是 | 服务器时间戳（毫秒） |
| `timezone_offset` | Number | 否 | 时区偏移（分钟） |

#### firmware 对象
| 字段 | 类型 | 必需 | 描述 |
|------|------|------|------|
| `version` | String | 是 | 固件版本号 |
| `url` | String | 是 | 固件下载地址 |
| `force` | Number | 否 | 强制更新标志，1=强制更新 |

## 2. 设备激活接口

### 2.1 请求参数

#### 请求 URL
```
{OTA_URL}/activate
```

#### 请求方法
**POST**

#### 请求头部
与版本检查接口相同的头部参数。

#### 请求体
```json
{
  "algorithm": "hmac-sha256",
  "serial_number": "设备序列号",
  "challenge": "服务器下发的挑战字符串",
  "hmac": "使用设备密钥计算的 HMAC 值"
}
```

### 2.2 HMAC 计算说明

1. **算法**: HMAC-SHA256
2. **密钥**: 使用 ESP32 的 HMAC Key0
3. **数据**: 服务器下发的 challenge 字符串
4. **输出**: 32 字节的 HMAC 值，转换为 64 位十六进制字符串

### 2.3 响应状态码

| 状态码 | 描述 |
|--------|------|
| 200 | 激活成功 |
| 202 | 激活处理中，需要等待 |
| 其他 | 激活失败 |

## 3. 版本比较逻辑

### 3.1 版本格式
支持语义化版本号格式，如：`1.2.3`

### 3.2 比较规则
1. 按 `.` 分割版本号为数字数组
2. 从左到右逐个比较数字
3. 如果某个位置的数字更大，则认为版本更新
4. 如果前缀相同但长度不同，较长的版本号被认为更新

### 3.3 强制更新
当服务器响应中 `firmware.force` 字段为 `1` 时，无论版本比较结果如何，都将触发固件更新。

## 4. 固件升级流程

### 4.1 升级步骤
1. 从指定 URL 下载固件文件
2. 验证固件头部信息
3. 检查新固件版本与当前版本
4. 写入 OTA 分区
5. 设置启动分区
6. 重启设备

### 4.2 进度回调
升级过程中会触发回调函数，提供：
- 下载进度百分比
- 当前下载速度 (字节/秒)

## 5. 错误处理

### 5.1 常见错误情况
- HTTP 连接失败
- JSON 解析失败
- 固件验证失败
- OTA 分区写入失败
- 激活 HMAC 计算失败

### 5.2 错误恢复
- OTA 升级失败时会自动回滚到原分区
- 首次启动时标记固件为有效状态

## 6. 安全特性

### 6.1 设备认证
- 基于设备 MAC 地址和 UUID 的身份识别
- 支持基于硬件 HMAC 密钥的设备激活

### 6.2 固件验证
- 下载过程中验证固件头部
- ESP32 硬件级固件签名验证

## 7. 配置说明

### 7.1 OTA URL 配置
1. 优先使用 WiFi 设置中的 `ota_url` 配置
2. 如果未配置，使用编译时的 `CONFIG_OTA_URL` 默认值

### 7.2 序列号支持
- 从 ESP32 eFuse 用户数据区读取 32 字节序列号
- 支持基于序列号的设备激活和认证 