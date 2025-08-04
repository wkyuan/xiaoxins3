# 小新S3 智能设备项目

## 项目简介

这是一个基于ESP32-S3的智能设备项目，支持多种硬件配置和功能模块。

## 硬件支持

项目支持多种开发板配置：
- xiaoxin-esp32s3
- xiaoxin-esp32s3-touch
- xiaoxin-esp32s3-096
- xiaoxin-esp32s3-028
- xiaoxin-esp32s3-154
- xiaoxin-esp32s3-4G
- xiaoxin-esp32s3-154-4G
- xiaoxin-esp32s3-154-wifi
- xiaoxin-zhi
- xiaoxin-esp32s3-lanka

## 蓝牙配网说明

### 配网流程

1. **设备启动**
   - 设备上电后，LED指示灯会显示当前状态
   - 如果未配置WiFi，设备会进入配网模式

2. **进入配网模式**
   - 长按设备上的配网按钮（通常为GPIO0）
   - LED指示灯会闪烁表示进入配网模式
   - 设备会启动蓝牙广播

3. **手机连接**
   - 打开手机蓝牙
   - 搜索并连接设备（设备名称通常为"Xiaoxin_XXXX"）
   - 连接成功后，设备会发送配网信息

4. **配置WiFi**
   - 通过蓝牙发送WiFi SSID和密码
   - 设备会尝试连接指定的WiFi网络
   - 连接成功后，LED指示灯会显示成功状态

5. **配网完成**
   - 设备保存WiFi配置信息
   - 重启后会自动连接已配置的WiFi网络

### 蓝牙配网API

```c
// 启动蓝牙配网
esp_err_t start_ble_provisioning(void);

// 停止蓝牙配网
esp_err_t stop_ble_provisioning(void);

// 获取配网状态
bool is_provisioning_active(void);
```

## LED指示灯说明

### LED状态定义

| LED状态 | 含义 | 说明 |
|---------|------|------|
| 常亮 | 正常工作 | 设备正常运行，已连接WiFi |
| 慢闪 | 配网模式 | 等待WiFi配置 |
| 快闪 | 连接中 | 正在连接WiFi网络 |
| 双闪 | 错误状态 | 连接失败或系统错误 |
| 熄灭 | 休眠模式 | 设备进入低功耗模式 |

### LED控制API

```c
// 设置LED状态
esp_err_t led_set_state(led_state_t state);

// LED状态枚举
typedef enum {
    LED_OFF = 0,        // 熄灭
    LED_ON,             // 常亮
    LED_SLOW_BLINK,     // 慢闪
    LED_FAST_BLINK,     // 快闪
    LED_DOUBLE_BLINK    // 双闪
} led_state_t;
```

### 硬件配置

不同开发板的LED配置可能不同：

#### xiaoxin-esp32s3
- LED引脚：GPIO2
- 支持RGB LED控制

#### xiaoxin-zhi
- LED引脚：GPIO2
- 支持单色LED和RGB LED

#### xiaoxin-esp32s3-lanka
- LED引脚：GPIO2
- 支持WS2812 RGB LED
- 详细说明请参考：`main/boards/xiaoxin-esp32s3-lanka/README_LED_CONTROL.md`

## 编译和烧录

### 环境要求
- ESP-IDF v5.0或更高版本
- Python 3.7+

### 编译步骤
```bash
# 配置项目
idf.py menuconfig

# 编译项目
idf.py build

# 烧录到设备
idf.py flash

# 监视串口输出
idf.py monitor
```

### 配置选项

在`idf.py menuconfig`中可以配置：
- 开发板类型（BOARD_TYPE）
- 语言设置（LANGUAGE）
- 音频处理器（USE_AUDIO_PROCESSOR）
- 唤醒词（USE_AFE_WAKE_WORD/USE_ESP_WAKE_WORD）

## 项目结构

```
xiaozhi1.7.6/
├── main/                    # 主程序代码
│   ├── boards/             # 开发板配置
│   ├── audio_codecs/       # 音频编解码器
│   ├── audio_processing/   # 音频处理
│   ├── display/            # 显示模块
│   ├── led/                # LED控制
│   ├── protocols/          # 通信协议
│   └── iot/                # IoT功能
├── docs/                   # 文档
├── scripts/                # 工具脚本
└── partitions/             # 分区表
```

## 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 贡献

欢迎提交Issue和Pull Request来改进项目。

## 联系方式

如有问题，请通过GitHub Issues联系我们。 