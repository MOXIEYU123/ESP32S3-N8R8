#include "aht10.h"
#include "i2c_driver.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"

// 日志标签
static const char *TAG = "AHT10";

/**
 * @brief 初始化AHT10传感器
 * 
 * @param i2c_num I2C端口号
 * @return esp_err_t 成功返回ESP_OK，失败返回相应错误码
 */
esp_err_t aht10_init(i2c_port_t i2c_num) {
    esp_err_t ret;
    
    // 上电后等待至少40ms（AHT10要求）
    vTaskDelay(50 / portTICK_PERIOD_MS);
    
    ESP_LOGI(TAG, "开始初始化AHT10传感器...");
    
    // 首先检查传感器状态
    uint8_t status_data[1] = {0};
    ret = my_i2c_master_read(i2c_num, AHT10_ADDR, status_data, 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "读取状态失败，错误代码: %d", ret);
        return ret;
    }
    ESP_LOGI(TAG, "传感器状态: 0x%02X", status_data[0]);
    
    // 如果传感器未校准，发送初始化命令
    if (!(status_data[0] & 0x08)) {
        ESP_LOGI(TAG, "传感器需要初始化");
        
        // 使用i2c_master_cmd_begin直接发送初始化命令
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (AHT10_ADDR << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, 0xE1, true);  // 初始化命令
        i2c_master_write_byte(cmd, 0x08, true);  // 参数1
        i2c_master_write_byte(cmd, 0x00, true);  // 参数2
        i2c_master_stop(cmd);
        
        ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "初始化命令发送失败，错误代码: %d", ret);
            return ret;
        }
        
        // 等待初始化完成
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    ESP_LOGI(TAG, "AHT10初始化成功");
    return ESP_OK;
}

/**
 * @brief 从AHT10读取温湿度数据
 * 
 * @param i2c_num I2C端口号
 * @param data 存储温湿度数据的结构体指针
 * @return esp_err_t 成功返回ESP_OK，失败返回相应错误码
 */
esp_err_t aht10_read_data(i2c_port_t i2c_num, aht10_data_t *data) {
    if (data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret;
    // 测量命令数据 - 触发测量命令
    uint8_t measure_cmd[3] = {0xAC, 0x33, 0x00};
    // 存储读取到的原始数据
    uint8_t read_data[6] = {0};
    
    // 发送测量命令
    ret = my_i2c_master_write(i2c_num, AHT10_ADDR, measure_cmd, sizeof(measure_cmd));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "发送测量命令失败");
        return ret;
    }
    
    // 等待测量完成
    vTaskDelay(100 / portTICK_PERIOD_MS);
    
    // 读取测量结果
    ret = my_i2c_master_read(i2c_num, AHT10_ADDR, read_data, sizeof(read_data));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "读取数据失败");
        return ret;
    }
    
    // 检查数据是否有效（第0位的第7位为0表示数据有效）
    // 数据有效性判断原理：
    // AHT10 传感器的响应数据中，第一个字节（read_data[0]）包含了状态信息
    // 其中最高位（第 7 位，从 0 开始计数）是数据就绪标志位
    // 当该位为 0 时，表示测量完成且数据有效；为 1 时，表示数据尚未准备好或无效
    // 位运算解析：
    // 0x80 是十六进制数，对应的二进制是 10000000
    // read_data[0] & 0x80 这一运算会保留read_data[0]的第 7 位，其他位都变为 0
    // 如果结果为 0（即!(read_data[0] & 0x80)为真），说明第 7 位是 0，数据有效
    if (!(read_data[0] & 0x80)) {
        // 计算湿度值
        // 湿度原始数据为20位：read_data[1]的8位 + read_data[2]的8位 + read_data[3]的高4位
        uint32_t humidity_raw = ((uint32_t)read_data[1] << 12) | 
                               ((uint32_t)read_data[2] << 4) | 
                               ((read_data[3] >> 4) & 0x0F);
        // 转换为湿度百分比（0-100%）
        data->humidity = (humidity_raw * 100.0f) / 0x100000;
        
        // 计算温度值
        // 温度原始数据为20位：read_data[3]的低4位 + read_data[4]的8位 + read_data[5]的8位
        uint32_t temp_raw = (((uint32_t)read_data[3] & 0x0F) << 16) | 
                           ((uint32_t)read_data[4] << 8) | 
                           read_data[5];
        // 转换为温度（-40℃ 至 85℃）
        data->temperature = (temp_raw * 200.0f) / 0x100000 - 50;
        
        ESP_LOGI(TAG, "数据读取成功 - 温度: %.2f°C, 湿度: %.2f%%", 
                 data->temperature, data->humidity);
        return ESP_OK;
    } else {
        ESP_LOGE(TAG, "数据无效");
        return ESP_FAIL;
    }
};
