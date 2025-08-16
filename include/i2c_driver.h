#ifndef __I2C_DRIVER_H__
#define __I2C_DRIVER_H__

#include "driver/i2c.h"
#include "esp_err.h"

/**
 * @brief 初始化I2C控制器
 * 
 * @param config I2C配置参数
 * @return esp_err_t 成功返回ESP_OK，失败返回相应错误码
 */
esp_err_t i2c_master_init(i2c_port_t i2c_num, i2c_config_t *config);

/**
 * @brief 向I2C设备写入数据
 * 
 * @param dev_addr I2C设备地址
 * @param data 要写入的数据
 * @param data_len 数据长度
 * @return esp_err_t 成功返回ESP_OK，失败返回相应错误码
 */
esp_err_t my_i2c_master_write(i2c_port_t i2c_num, uint8_t dev_addr, const uint8_t *data, size_t data_len);

/**
 * @brief 从I2C设备读取数据
 * 
 * @param dev_addr I2C设备地址
 * @param data 读取到的数据存储地址
 * @param data_len 数据长度
 * @return esp_err_t 成功返回ESP_OK，失败返回相应错误码
 */
esp_err_t my_i2c_master_read(i2c_port_t i2c_num, uint8_t dev_addr, uint8_t *data, size_t data_len);

#endif