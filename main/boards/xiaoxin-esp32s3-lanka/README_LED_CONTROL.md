# Xiaoxin ESP32S3 Lanka 主板 LED控制说明

## 概述

Xiaoxin ESP32S3 Lanka 主板集成了完整的LED控制系统，支持多种LED效果和按钮控制功能。本主板基于ESP32-S3芯片，提供了三个独立的LED控制通道，支持常亮、闪烁、呼吸灯等多种效果。

## 硬件配置

### LED引脚配置
- **LED1**: `BUILTIN_LED_GPIO1` + `LEDC_CHANNEL_0`
- **LED2**: `BUILTIN_LED_GPIO2` + `LEDC_CHANNEL_1`  
- **LED3**: `BUILTIN_LED_GPIO3` + `LEDC_CHANNEL_2`

### 按钮配置
- **BOOT按钮**: `BOOT_BUTTON_GPIO` - 系统启动和网络切换
- **电源键**: `POWER_KEY_GPIO` - 电源管理和LED控制
- **音量+**: `VOLUME_UP_BUTTON_GPIO` - 音量调节和LED效果切换
- **音量-**: `VOLUME_DOWN_BUTTON_GPIO` - 音量调节和LED效果切换

## LED控制功能

### 1. 基础LED控制

#### 开关控制
```cpp
// 在xiaoxin_esp32s3_lanka.cc中
void SetLedState(int led_index, bool turn_on, uint8_t brightness = 50);

// 使用示例
SetLedState(1, true, 100);  // LED1 100%亮度开启
SetLedState(2, true, 50);   // LED2 50%亮度开启
SetLedState(3, false);      // LED3 关闭
```

#### 亮度调节
```cpp
// 使用GpioLed的SetBrightness方法
led1->SetBrightness(100);  // 设置LED1为100%亮度
led2->SetBrightness(50);   // 设置LED2为50%亮度
led3->SetBrightness(25);   // 设置LED3为25%亮度
```

### 2. 呼吸灯特效

#### 硬件呼吸灯（推荐）
```cpp
// 使用GpioLed内置的呼吸灯功能
led1->OnStateChanged();  // 根据设备状态自动启动呼吸灯

// 在Listening状态下，LED会自动进入呼吸灯模式
// 亮度范围：5%-100%
// 周期：1秒
```

#### 软件呼吸灯（自定义）
```cpp
// 创建自定义呼吸灯任务
void StartLedBreathingEffect(int led_index, uint8_t min_brightness = 5, 
                           uint8_t max_brightness = 100, int cycle_ms = 2000);

// 使用示例
StartLedBreathingEffect(1, 5, 100, 3000);  // LED1: 3秒周期，5-100%亮度
StartLedBreathingEffect(2, 10, 80, 2000);  // LED2: 2秒周期，10-80%亮度
```

### 3. 闪烁效果

#### 单次闪烁
```cpp
led1->BlinkOnce();  // LED1 闪烁一次
```

#### 多次闪烁
```cpp
led1->Blink(3, 200);  // LED1 闪烁3次，间隔200ms
```

#### 连续闪烁
```cpp
led1->StartContinuousBlink(500);  // LED1 连续闪烁，间隔500ms
```

## 按钮控制LED

### 1. BOOT按钮控制
```cpp
// 单击：切换聊天状态，LED状态相应变化
boot_button_.OnClick([this]() {
    ESP_LOGI(TAG, "boot_button_ OnClick");
    auto& app = Application::GetInstance();
    app.ToggleChatState();  // 这会触发LED状态变化
});

// 双击：切换网络类型
boot_button_.OnDoubleClick([this]() {
    ESP_LOGI(TAG, "boot_button_ OnDoubleClick");
    SwitchNetworkType();
});
```

### 2. 电源键长按控制
```cpp
// 长按电源键：关闭特定GPIO
power_key_button_.OnLongPress([this]() {
    ESP_LOGI(TAG, "power_key_button_ OnLongPress");
    gpio_reset_pin(GPIO_NUM_3);
    gpio_set_direction(GPIO_NUM_3, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_3, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    
    gpio_reset_pin(GPIO_NUM_38);
    gpio_set_direction(GPIO_NUM_38, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_38, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
});
```

### 3. 音量按钮控制
```cpp
// 音量+按钮：调节音量和唤醒LED
volume_up_button_.OnClick([this]() {
    auto codec = GetAudioCodec();
    auto volume = codec->output_volume() + 10;
    if (volume > 100) volume = 100;
    codec->SetOutputVolume(volume);
    // LED会相应亮起表示音量调节
});

// 音量+长按：最大音量
volume_up_button_.OnLongPress([this]() {
    GetAudioCodec()->SetOutputVolume(100);
    // LED全亮表示最大音量
});

// 音量-按钮：调节音量
volume_down_button_.OnClick([this]() {
    auto codec = GetAudioCodec();
    auto volume = codec->output_volume() - 10;
    if (volume < 0) volume = 0;
    codec->SetOutputVolume(volume);
    // LED亮度相应降低
});

// 音量-长按：静音
volume_down_button_.OnLongPress([this]() {
    GetAudioCodec()->SetOutputVolume(0);
    // LED关闭表示静音
});
```

## 设备状态与LED对应关系

### 状态映射
| 设备状态 | LED行为 | 亮度 | 效果 |
|---------|---------|------|------|
| `Starting` | 连续闪烁 | 50% | 100ms间隔闪烁 |
| `WifiConfiguring` | 连续闪烁 | 50% | 500ms间隔闪烁 |
| `Idle` | 常亮 | 5% | 低亮度常亮 |
| `Connecting` | 常亮 | 50% | 中等亮度常亮 |
| `Listening/AudioTesting` | **呼吸灯** | 10%-100% | 根据语音检测调整亮度 |
| `Speaking` | 常亮 | 75% | 高亮度常亮 |
| `Upgrading` | 连续闪烁 | 25% | 100ms间隔闪烁 |
| `Activating` | 连续闪烁 | 35% | 500ms间隔闪烁 |

### 状态变化触发
```cpp
void OnStateChanged() {
    auto& app = Application::GetInstance();
    auto device_state = app.GetDeviceState();
    
    switch (device_state) {
        case kDeviceStateListening:
        case kDeviceStateAudioTesting:
            if (app.IsVoiceDetected()) {
                SetBrightness(HIGH_BRIGHTNESS);  // 语音检测时高亮度
            } else {
                SetBrightness(LOW_BRIGHTNESS);   // 无语音时低亮度
            }
            StartFadeTask();  // 启动呼吸灯效果
            break;
        // ... 其他状态处理
    }
}
```

## 长按时间配置

### 按钮长按时间设置
```cpp
// 在构造函数中配置按钮长按时间
XIAOXINESP32S3LANKA() : 
    boot_button_(BOOT_BUTTON_GPIO, true, 5000, 500),      // 长按5秒
    power_key_button_(POWER_KEY_GPIO, true, 5000, 500),    // 长按5秒
    volume_up_button_(VOLUME_UP_BUTTON_GPIO, false, 1500, 500),   // 长按1.5秒
    volume_down_button_(VOLUME_DOWN_BUTTON_GPIO, false, 1500, 500) // 长按1.5秒
```

### 修改长按时间
```cpp
// 修改按钮长按时间（在config.h中）
#define BOOT_BUTTON_LONG_PRESS_TIME    5000   // 5秒
#define POWER_KEY_LONG_PRESS_TIME      5000   // 5秒
#define VOLUME_UP_LONG_PRESS_TIME      1500   // 1.5秒
#define VOLUME_DOWN_LONG_PRESS_TIME    1500   // 1.5秒
```

## 实际使用示例

### 1. 初始化LED系统
```cpp
// 在构造函数中
InitializeLeds();  // 初始化三个LED控制器

// 测试LED功能
led1 = new GpioLed(BUILTIN_LED_GPIO1);
led1->SetBrightness(100);
led1->TurnOn();
led1->StartContinuousBlink(1000);  // 启动闪烁效果
```

### 2. 按钮控制LED
```cpp
// 音量按钮控制LED亮度
volume_up_button_.OnClick([this]() {
    // 增加音量时，LED亮度也增加
    static uint8_t led_brightness = 50;
    led_brightness = (led_brightness + 10) > 100 ? 100 : led_brightness + 10;
    led1->SetBrightness(led_brightness);
    led1->TurnOn();
});

volume_down_button_.OnClick([this]() {
    // 减少音量时，LED亮度也减少
    static uint8_t led_brightness = 50;
    led_brightness = (led_brightness < 10) ? 0 : led_brightness - 10;
    led1->SetBrightness(led_brightness);
    if (led_brightness == 0) {
        led1->TurnOff();
    } else {
        led1->TurnOn();
    }
});
```

### 3. 呼吸灯效果
```cpp
// 在Listening状态下自动启动呼吸灯
case kDeviceStateListening:
    if (app.IsVoiceDetected()) {
        SetBrightness(HIGH_BRIGHTNESS);
    } else {
        SetBrightness(LOW_BRIGHTNESS);
    }
    StartFadeTask();  // 启动硬件呼吸灯
    break;
```

## 技术实现细节

### 1. 硬件PWM控制
- **PWM频率**: 4kHz
- **分辨率**: 13位 (0-8191)
- **渐变时间**: 1000ms
- **通道配置**: 每个LED使用独立LEDC通道

### 2. 呼吸灯算法
```cpp
void OnFadeEnd() {
    fade_up_ = !fade_up_;  // 切换方向
    ledc_set_fade_with_time(ledc_channel_.speed_mode,
                            ledc_channel_.channel, 
                            fade_up_ ? LEDC_DUTY : 0, 
                            LEDC_FADE_TIME);
    ledc_fade_start(ledc_channel_.speed_mode,
                    ledc_channel_.channel, 
                    LEDC_FADE_NO_WAIT);
}
```

### 3. 线程安全
- 使用互斥锁保护LED操作
- 支持多任务并发访问
- 正确的资源初始化和清理

## 故障排除

### 常见问题
1. **LED不亮**: 检查GPIO配置和LEDC通道设置
2. **呼吸灯不平滑**: 调整LEDC_FADE_TIME参数
3. **按钮无响应**: 检查按钮GPIO配置和长按时间设置
4. **闪烁异常**: 检查定时器配置和任务优先级

### 调试方法
```cpp
// 启用详细日志
ESP_LOGI(TAG, "LED1: 100%% brightness ON");
ESP_LOGI(TAG, "Button long press detected");
ESP_LOGI(TAG, "Breathing effect started");
```

## 配置参数

### LED配置
```cpp
#define LEDC_DUTY              (8191)      // PWM占空比
#define LEDC_FADE_TIME         (1000)      // 渐变时间(ms)
#define DEFAULT_BRIGHTNESS     50          // 默认亮度
#define HIGH_BRIGHTNESS        100         // 高亮度
#define LOW_BRIGHTNESS         10          // 低亮度
```

### 按钮配置
```cpp
#define BOOT_BUTTON_GPIO       GPIO_NUM_0
#define POWER_KEY_GPIO         GPIO_NUM_1
#define VOLUME_UP_BUTTON_GPIO  GPIO_NUM_2
#define VOLUME_DOWN_BUTTON_GPIO GPIO_NUM_3
```

这个LED控制系统为Xiaoxin ESP32S3 Lanka主板提供了完整的视觉反馈功能，支持多种交互模式和状态指示。 