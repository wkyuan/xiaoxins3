# LED控制功能使用说明

## 概述
本项目实现了基于 `gpio_led.cc` 的三个LED控制功能，支持：
- LED常亮/关闭
- LED亮度调节
- LED闪烁效果
- LED呼吸灯效果

## LED配置
- **LED1**: `BUILTIN_LED_GPIO1` + `LEDC_CHANNEL_0`
- **LED2**: `BUILTIN_LED_GPIO2` + `LEDC_CHANNEL_1`  
- **LED3**: `BUILTIN_LED_GPIO3` + `LEDC_CHANNEL_2`

## 功能接口

### 1. 基础LED控制
```cpp
// 设置LED状态
SetLedState(int led_index, bool turn_on, uint8_t brightness = 50);

// 示例：
SetLedState(1, true, 100);  // LED1 100%亮度常亮
SetLedState(2, true, 50);   // LED2 50%亮度常亮
SetLedState(3, false);      // LED3 关闭
```

### 2. LED闪烁效果
```cpp
// 启动LED闪烁
StartLedBlink(int led_index, int times = -1, int interval_ms = 500);

// 示例：
StartLedBlink(1, 3, 200);   // LED1 闪烁3次，间隔200ms
StartLedBlink(2, -1, 500);  // LED2 连续闪烁，间隔500ms
```

### 3. LED呼吸灯效果
```cpp
// 启动LED呼吸灯效果
StartLedBreathingEffect(int led_index, uint8_t min_brightness = 5, 
                       uint8_t max_brightness = 100, int cycle_ms = 2000);

// 停止呼吸灯效果
StopLedBreathingEffect();

// 示例：
StartLedBreathingEffect(1, 5, 100, 3000);  // LED1 3秒周期，5-100%亮度
StartLedBreathingEffect(2, 10, 80, 2000);  // LED2 2秒周期，10-80%亮度
```

## 使用示例

### 基本使用
```cpp
// 初始化LED控制器
InitializeLeds();

// 设置LED状态
SetLedState(1, true, 100);  // LED1 100%亮度
SetLedState(2, true, 50);   // LED2 50%亮度
SetLedState(3, false);      // LED3 关闭
```

### 呼吸灯效果
```cpp
// 启动呼吸灯
StartLedBreathingEffect(1, 5, 100, 3000);  // LED1 呼吸灯
StartLedBreathingEffect(2, 10, 80, 2000);  // LED2 呼吸灯

// 停止呼吸灯
StopLedBreathingEffect();
```

### 闪烁效果
```cpp
// 闪烁3次
StartLedBlink(1, 3, 200);

// 连续闪烁
StartLedBlink(2, -1, 500);
```

## 技术实现

### 1. 硬件PWM控制
- 使用ESP32的LEDC硬件PWM模块
- 支持13位PWM分辨率
- 频率：4kHz
- 支持渐变效果

### 2. 呼吸灯算法
- 使用FreeRTOS任务实现
- 50ms步进更新
- 支持自定义亮度范围和周期
- 平滑的亮度渐变

### 3. 线程安全
- 使用互斥锁保护共享资源
- 支持多任务并发访问
- 正确的资源管理

## 注意事项

1. **LEDC通道冲突**: 确保不同LED使用不同的LEDC通道
2. **任务优先级**: 呼吸灯任务优先级为5
3. **内存管理**: 自动清理LED资源
4. **电源管理**: 支持低功耗模式

## 故障排除

### 常见问题
1. **LED不亮**: 检查GPIO配置和LEDC通道设置
2. **呼吸灯不平滑**: 调整步进时间和更新频率
3. **闪烁异常**: 检查定时器配置和任务优先级

### 调试方法
```cpp
// 启用调试日志
ESP_LOGI(TAG, "LED1: 100%% brightness ON");
ESP_LOGI(TAG, "LED2: Breathing effect started");
``` 