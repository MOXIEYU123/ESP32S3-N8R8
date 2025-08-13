#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#include "i2c_driver.h"

// 日志标签
static const char *TAG = "I2C_DRIVER";

/**
 * @brief 初始化I2C控制器
 * 
 * @param config I2C配置参数
 * @return esp_err_t 成功返回ESP_OK，失败返回相应错误码
 */
esp_err_t i2c_master_init(i2c_port_t i2c_num, i2c_config_t *config) {
    if (config == NULL) {
        ESP_LOGE(TAG, "配置参数不能为空");
        return ESP_ERR_INVALID_ARG;
    }
    
    // 配置I2C参数
    i2c_config_t i2c_conf = {
        .mode             = I2C_MODE_MASTER,      // 设置I2C工作模式为主机模式
        .sda_io_num       = config->sda_io_num,    // 设置SDA引脚号，从传入的配置参数中获取
        .scl_io_num       = config->scl_io_num,    // 设置SCL引脚号，从传入的配置参数中获取
        .sda_pullup_en    = config->sda_pullup_en, // 设置SDA引脚是否启用上拉电阻，从传入的配置参数中获取
        .scl_pullup_en    = config->scl_pullup_en, // 设置SCL引脚是否启用上拉电阻，从传入的配置参数中获取
        .master.clk_speed = config->master.clk_speed,     // 设置主机模式下的时钟频率，从传入的配置参数中获取
    };
    
    // 调用i2c_param_config函数配置指定I2C控制器的参数
    esp_err_t ret = i2c_param_config(i2c_num, &i2c_conf);
    // 检查参数配置是否成功
    if (ret != ESP_OK) {
        // 若配置失败，记录错误日志并返回错误码
        ESP_LOGE(TAG, "I2C参数配置失败: %d", ret);
        return ret;
    }
    
    // 安装I2C驱动
    ret = i2c_driver_install(i2c_num, i2c_conf.mode, 
                            0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C驱动安装失败: %d", ret);
        return ret;
    }
    
    ESP_LOGI(TAG, "I2C端口 %lu 初始化成功，频率: %lu Hz", 
             (unsigned long)i2c_num, (unsigned long)config->master.clk_speed);
    return ESP_OK;
}

/**
 * @brief I2C写入数据
 * 
 * @param i2c_num I2C端口号
 * @param dev_addr 设备地址
 * @param data 要写入的数据
 * @param data_len 数据长度
 * @return esp_err_t 成功返回ESP_OK，失败返回相应错误码
 */
 esp_err_t my_i2c_master_write(i2c_port_t i2c_num, uint8_t dev_addr, 
                          const uint8_t *data, size_t data_len) {
    // 检查输入参数是否合法，若数据指针为空或数据长度为0，
    // 则认为输入参数无效，返回参数错误码
    if (data == NULL || data_len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // 创建I2C命令链
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // 发送设备地址和写位
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
    // 发送数据
    i2c_master_write(cmd, data, data_len, true);
    i2c_master_stop(cmd);
    
    // 执行I2C命令
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_PERIOD_MS);
    
    // 删除命令链
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C写入失败，设备地址: 0x%02X, 错误代码: %d", dev_addr, ret);
    }
    
    return ret;
}

/**
 * @brief I2C读取数据
 * 
 * @param i2c_num I2C端口号
 * @param dev_addr 设备地址
 * @param data 存储读取数据的缓冲区
 * @param data_len 要读取的数据长度
 * @return esp_err_t 成功返回ESP_OK，失败返回相应错误码
 */
esp_err_t my_i2c_master_read(i2c_port_t i2c_num, uint8_t dev_addr, 
                         uint8_t *data, size_t data_len) {
    if (data == NULL || data_len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // 创建I2C命令链
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // 发送设备地址和读位
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true);
    
    // 读取数据
    if (data_len > 1) {
        // 读取前data_len-1个字节，发送ACK
        i2c_master_read(cmd, data, data_len - 1, I2C_MASTER_ACK);
    }
    // 读取最后1个字节，发送NACK
    i2c_master_read_byte(cmd, &data[data_len - 1], I2C_MASTER_NACK);
    
    i2c_master_stop(cmd);
    
    // 执行I2C命令
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_PERIOD_MS);
    
    // 删除命令链
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C读取失败，设备地址: 0x%02X, 错误代码: %d", dev_addr, ret);
    }
    
    return ret;
}
