#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2c_driver.h"
#include "aht10.h"

// 日志标签
static const char *TAG = "MAIN";

// 硬件配置
#define I2C_MASTER_NUM I2C_NUM_0       /* I2C端口号 */
#define I2C_MASTER_SDA_IO 3            /* GPIO3作为SDA */
#define I2C_MASTER_SCL_IO 2            /* GPIO2作为SCL */
#define I2C_MASTER_FREQ_HZ 400000      /* I2C频率 400kHz */

/**
 * @brief 温湿度读取任务
 * 
 * @param pvParameters 任务参数
 */
static void aht10_task(void *pvParameters) {
    aht10_data_t data;
    i2c_port_t i2c_num = I2C_MASTER_NUM;
    
    // 初始化AHT10
    if (aht10_init(i2c_num) != ESP_OK) {
        ESP_LOGE(TAG, "AHT10初始化失败，退出任务");
        vTaskDelete(NULL);
    }
    
    while (1) {
        // 读取温湿度数据
        if (aht10_read_data(i2c_num, &data) == ESP_OK) {
            printf("温度: %.2f°C, 湿度: %.2f%%\n", 
                   data.temperature, data.humidity);
        } else {
            ESP_LOGE(TAG, "读取AHT10数据失败");
        }
        
        // 每2秒读取一次
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "AHT10温湿度监测程序启动");
    
    // 配置I2C参数
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };
    
    // 初始化I2C
    esp_err_t ret = i2c_master_init(I2C_MASTER_NUM, &i2c_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C初始化失败，错误代码: %d", ret);
        return;
    }
    
    // 扫描I2C设备
    ESP_LOGI(TAG, "扫描I2C总线设备...");
    uint8_t found_devices = 0;
    for (uint8_t addr = 0x01; addr < 0x7F; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        
        esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 50 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "发现设备: 0x%02X", addr);
            found_devices++;
        }
    }
    
    if (found_devices == 0) {
        ESP_LOGW(TAG, "未在I2C总线上发现任何设备，请检查连接");
    } else {
        ESP_LOGI(TAG, "共发现 %d 个设备", found_devices);
    }
    
    // 创建AHT10读取任务
    xTaskCreate(aht10_task, "aht10_task", 2048, NULL, 5, NULL);
}
