#ifndef __AHT10_H__
#define __AHT10_H__

#include "driver/i2c.h"
#include "esp_err.h"

// AHT10 I2C设备地址
#define AHT10_ADDR 0x38

// AHT10命令定义
#define AHT10_CMD_INIT 0xE1    // 初始化命令
#define AHT10_CMD_MEASURE 0xAC // 测量命令
#define AHT10_CMD_RESET 0xBA   // 软复位命令

// 温湿度数据结构体
typedef struct {
    float temperature; // 温度，单位：摄氏度
    float humidity;    // 湿度，单位：百分比
} aht10_data_t;

/**
 * @brief 初始化AHT10传感器
 * 
 * @param i2c_num I2C端口号
 * @return esp_err_t 成功返回ESP_OK，失败返回相应错误码
 */
esp_err_t aht10_init(i2c_port_t i2c_num);

/**
 * @brief 从AHT10读取温湿度数据
 * 
 * @param i2c_num I2C端口号
 * @param data 存储温湿度数据的结构体指针
 * @return esp_err_t 成功返回ESP_OK，失败返回相应错误码
 */
esp_err_t aht10_read_data(i2c_port_t i2c_num, aht10_data_t *data);


#endif