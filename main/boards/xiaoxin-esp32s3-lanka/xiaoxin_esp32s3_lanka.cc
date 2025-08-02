#include "dual_network_board.h"
#include "audio_codecs/es8311_audio_codec.h"
#include "display/lcd_display.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "iot/thing_manager.h"
#include "led/single_led.h"
#include "power_manager.h"
#include <wifi_station.h>
#include <esp_log.h>
#include <esp_efuse_table.h>
#include <driver/i2c_master.h>
#include "assets/lang_config.h"
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_gc9a01.h>
#include "mcp_server.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

#define TAG "XIAOXINESP32S3LANKA"
#include "led/gpio_led.h"



LV_FONT_DECLARE(font_puhui_20_4);
LV_FONT_DECLARE(font_awesome_20_4);




class XIAOXINESP32S3LANKA : public DualNetworkBoard {
private:
    i2c_master_bus_handle_t codec_i2c_bus_;
    Button boot_button_;
    Button power_key_button_;
    Button volume_up_button_;
    Button volume_down_button_;
    TaskHandle_t breathing_led_task_handle_ = nullptr;
    GpioLed* led1 = nullptr;
    GpioLed* led2 = nullptr;
    GpioLed* led3 = nullptr;
    PowerManager* power_manager_;
    esp_lcd_panel_io_handle_t panel_io_ = nullptr;
    esp_lcd_panel_handle_t panel_ = nullptr;

    
    void InitializePowerManager() {

        gpio_reset_pin(ADC_PA_PIN);
        gpio_set_direction(ADC_PA_PIN, GPIO_MODE_OUTPUT);     // ADC_CTRL 输出模式
        gpio_set_level(ADC_PA_PIN, 1); // 打开开关，让 VBAT 通过分压电路接入 ADC
        vTaskDelay(pdMS_TO_TICKS(10)); // 等待电压稳定
        power_manager_ = new PowerManager(CHRG_PIN);
        power_manager_->OnChargingStatusChanged([this](bool is_charging) {
            if (is_charging) {
                ESP_LOGI("PowerManager", "Charging");
                //power_save_timer_->SetEnabled(false);
                
            } else {
                ESP_LOGI("PowerManager", "Discharging");
                //power_save_timer_->SetEnabled(true);

            }
        });
    }
    void InitializeCodecI2c() {
        // Initialize I2C peripheral
        i2c_master_bus_config_t i2c_bus_cfg = {
            .i2c_port = I2C_NUM_0,
            .sda_io_num = AUDIO_CODEC_I2C_SDA_PIN,
            .scl_io_num = AUDIO_CODEC_I2C_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = {
                .enable_internal_pullup = 1,
            },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &codec_i2c_bus_));
    }


    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            ESP_LOGI(TAG, "boot_button_ OnClick");
            //power_save_timer_->WakeUp();
            auto& app = Application::GetInstance();
            if (GetNetworkType() == NetworkType::WIFI) {
                if (app.GetDeviceState() == kDeviceStateStarting && !WifiStation::GetInstance().IsConnected()) {
                    // cast to WifiBoard
                    auto& wifi_board = static_cast<WifiBoard&>(GetCurrentBoard());
                    wifi_board.ResetWifiConfiguration();
                }
            }
            app.ToggleChatState();
        });
        boot_button_.OnDoubleClick([this]() {
            ESP_LOGI(TAG, "boot_button_ OnDoubleClick");
            SwitchNetworkType();
        });
        // power_key_button_.OnClick([this]() {
        //     ESP_LOGI(TAG, "power_key_button_ OnClick");
        //     if (GetBacklight()->brightness() < 100) {
        //         GetBacklight()->SetBrightness(100);
        //         gpio_reset_pin(GPIO_NUM_38);
        //         gpio_set_direction(GPIO_NUM_38, GPIO_MODE_OUTPUT);
        //         gpio_set_level(GPIO_NUM_38, 1);
        //         vTaskDelay(pdMS_TO_TICKS(10)); // 等待电平稳定（可选）
        //     } else {
        //         GetBacklight()->SetBrightness(0);
        //         gpio_reset_pin(GPIO_NUM_38);
        //         gpio_set_direction(GPIO_NUM_38, GPIO_MODE_OUTPUT);
        //         gpio_set_level(GPIO_NUM_38, 0);
        //         vTaskDelay(pdMS_TO_TICKS(10)); // 等待电平稳定（可选）
        //     }
        // });
        power_key_button_.OnLongPress([this]() {
            ESP_LOGI(TAG, "power_key_button_ OnLongPress");
            gpio_reset_pin(GPIO_NUM_3);
            gpio_set_direction(GPIO_NUM_3, GPIO_MODE_OUTPUT);
            gpio_set_level(GPIO_NUM_3, 0);
            vTaskDelay(pdMS_TO_TICKS(10)); // 等待电平稳定（可选）
            ESP_LOGI(TAG, "GPIO_NUM_3: %d", gpio_get_level(GPIO_NUM_3));
            gpio_reset_pin(GPIO_NUM_38);
            gpio_set_direction(GPIO_NUM_38, GPIO_MODE_OUTPUT);
            gpio_set_level(GPIO_NUM_38, 0);
            vTaskDelay(pdMS_TO_TICKS(10)); // 等待电平稳定（可选）
            ESP_LOGI(TAG, "GPIO_NUM_38: %d", gpio_get_level(GPIO_NUM_38));
        });
        volume_up_button_.OnClick([this]() {
            //power_save_timer_->WakeUp();
            auto codec = GetAudioCodec();
            auto volume = codec->output_volume() + 10;
            if (volume > 100) {
                volume = 100;
            }
            codec->SetOutputVolume(volume);
           
        });

        volume_up_button_.OnLongPress([this]() {
            //power_save_timer_->WakeUp();
            GetAudioCodec()->SetOutputVolume(100);

        });

        volume_down_button_.OnClick([this]() {
            //power_save_timer_->WakeUp();
            auto codec = GetAudioCodec();
            auto volume = codec->output_volume() - 10;
            if (volume < 0) {
                volume = 0;
            }
            codec->SetOutputVolume(volume);

        });

        volume_down_button_.OnLongPress([this]() {
            //power_save_timer_->WakeUp();
            GetAudioCodec()->SetOutputVolume(0);

        });
    }

    // 物联网初始化，添加对 AI 可见设备
    void InitializeIot() {
        auto& thing_manager = iot::ThingManager::GetInstance();
        thing_manager.AddThing(iot::CreateThing("Speaker")); 
        thing_manager.AddThing(iot::CreateThing("Screen"));   
        //thing_manager.AddThing(iot::CreateThing("Lamp"));  
    }

public:
XIAOXINESP32S3LANKA() :DualNetworkBoard(ML307_TX_PIN, ML307_RX_PIN, 4096), boot_button_(BOOT_BUTTON_GPIO,true,5000,500), power_key_button_(POWER_KEY_GPIO,true,5000,500), volume_up_button_(VOLUME_UP_BUTTON_GPIO,false,1500,500), volume_down_button_(VOLUME_DOWN_BUTTON_GPIO,false,1500,500) {  
        
        gpio_reset_pin(GPIO_NUM_3);
        gpio_set_direction(GPIO_NUM_3, GPIO_MODE_OUTPUT);
        gpio_set_level(GPIO_NUM_3, 1);
        vTaskDelay(pdMS_TO_TICKS(10)); // 等待电平稳定（可选）
        ESP_LOGI(TAG, "GPIO_NUM_3: %d", gpio_get_level(GPIO_NUM_3));
        gpio_reset_pin(GPIO_NUM_38);
        gpio_set_direction(GPIO_NUM_38, GPIO_MODE_OUTPUT);
        gpio_set_level(GPIO_NUM_38, 1);
        vTaskDelay(pdMS_TO_TICKS(10)); // 等待电平稳定（可选）
        ESP_LOGI(TAG, "GPIO_NUM_38: %d", gpio_get_level(GPIO_NUM_38));
        gpio_reset_pin(ML307_RST_PIN);
        gpio_set_direction(ML307_RST_PIN, GPIO_MODE_OUTPUT);
        gpio_set_level(ML307_RST_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(10)); // 等待电平稳定（可选）
        gpio_reset_pin(BUILTIN_LED_GPIO);
        gpio_set_direction(BUILTIN_LED_GPIO, GPIO_MODE_OUTPUT);
        gpio_set_level(BUILTIN_LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(10)); // 等待电平稳定（可选）
        gpio_reset_pin(BUILTIN_LED_GPIO1);
        gpio_set_direction(BUILTIN_LED_GPIO1, GPIO_MODE_OUTPUT);     // ADC_CTRL 输出模式
        gpio_set_level(BUILTIN_LED_GPIO1, 1); // 打开开关，让 VBAT 通过分压电路接入 ADC
        InitializePowerManager();
        InitializeCodecI2c();

        InitializeButtons();
        InitializeIot();

        auto& mcp_server = McpServer::GetInstance();
        led1=new GpioLed(BUILTIN_LED_GPIO1);
        led1->SetBrightness(100);
        led1->TurnOn();

        led1->StartContinuousBlink(1000); // 启动呼吸灯


        
        
        
    }

    // virtual Led* GetLed() override {
    //     static SingleLed led_strip(BUILTIN_LED_GPIO);
    //     return &led_strip;
    // }
    virtual bool GetBatteryLevel(int& level, bool& charging, bool& discharging) override {
        static bool last_discharging = false;
        charging = power_manager_->IsCharging();
        discharging = power_manager_->IsDischarging();
        if (discharging != last_discharging) {
            //power_save_timer_->SetEnabled(discharging);
            last_discharging = discharging;
        }
        level = power_manager_->GetBatteryLevel();
        return true;
    }

    

    virtual AudioCodec* GetAudioCodec() override {
        static Es8311AudioCodec audio_codec(codec_i2c_bus_, I2C_NUM_0, AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_GPIO_MCLK, AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS, AUDIO_I2S_GPIO_DOUT, AUDIO_I2S_GPIO_DIN,
            AUDIO_CODEC_PA_PIN, AUDIO_CODEC_ES8311_ADDR);
        return &audio_codec;
    }
};

DECLARE_BOARD(XIAOXINESP32S3LANKA);
