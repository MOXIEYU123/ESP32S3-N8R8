# ESP32S3-N8R8 SmartConfig 项目

基于ESP32-S3的WiFi SmartConfig配置项目，支持通过ESPTOUCH APP进行WiFi配置，集成AHT10传感器和RESTful API服务器。

## 📋 项目特性

- **SmartConfig 功能**: 通过ESPTOUCH APP快速配置WiFi连接
- **事件处理系统**: 完整的WiFi事件处理机制
- **AHT10传感器**: 温湿度数据采集
- **I2C通信**: 完整的I2C驱动支持
- **RESTful API**: HTTP Web服务器和API接口
- **文件系统**: SPIFFS和FAT文件系统支持
- **mDNS服务**: 局域网域名解析服务

## 🛠️ 硬件需求

- **ESP32-S3开发板**: N8R8 (8MB Flash, 8MB PSRAM)
- **AHT10传感器模块**: 温湿度传感器
- **USB数据线**: 用于烧录和调试
- **WiFi网络**: 2.4GHz频段支持

## 📦 软件需求

- **ESP-IDF**: v5.4.1 或更高版本
- **ESPTOUCH APP**: Android/iOS应用商店下载
- **开发环境**: VS Code + ESP-IDF插件

## 🚀 快速开始

### 1. 克隆项目
```bash
git clone https://github.com/MOXIEYU123/ESP32S3-N8R8.git
cd ESP32S3-N8R8
```

### 2. 配置项目
```bash
idf.py set-target esp32s3
idf.py menuconfig
```

### 3. 编译和烧录
```bash
# 编译项目
idf.py build

# 烧录到开发板 (替换COMx为你的串口号)
idf.py -p COMx flash

# 监控串口输出
idf.py -p COMx monitor
```

### 4. WiFi配置
1. 确保手机连接到目标WiFi网络 (2.4GHz频段)
2. 打开ESPTOUCH APP
3. 输入WiFi密码
4. 等待配置成功提示

## 📁 项目结构

```
ESP32S3-N8R8/
├── main/
│   ├── main.c              # 主程序入口
│   └── CMakeLists.txt      # 主组件配置
├── src/
│   ├── smartconfig.c       # SmartConfig实现
│   ├── event_handler.c     # WiFi事件处理
│   ├── i2c_driver.c        # I2C驱动
│   ├── aht10.c            # AHT10传感器驱动
│   ├── esp_rest.c          # REST API服务器
│   └── rest_server.c       # HTTP服务器实现
├── include/
│   ├── smartconfig.h
│   ├── event_handler.h
│   ├── i2c_driver.h
│   ├── aht10.h
│   └── web_server_handler.h
├── web-demo/               # Web前端Vue项目
├── CMakeLists.txt          # 项目配置
└── README.md              # 项目说明
```

## 🔧 API接口文档

### 1. SmartConfig API (`smartconfig.h`)

#### 数据类型
```c
// 事件回调类型
typedef void (*smartconfig_event_cb_t)(esp_event_base_t event_base, 
                                     int32_t event_id, 
                                     void* event_data);

// 事件标志
#define CONNECTED_BIT BIT0      // WiFi连接成功
#define ESPTOUCH_DONE_BIT BIT1   // SmartConfig完成
```

#### 函数接口

| 函数名 | 说明 | 参数 | 返回值 |
|--------|------|------|--------|
| `smartconfig_init()` | 初始化SmartConfig模块 | 无 | 无 |
| `smartconfig_task(void *parm)` | SmartConfig任务函数 | `parm`: 任务参数 | 无 |

#### 事件处理
- **SC_EVENT_GOT_SSID_PSWD**: 接收到WiFi配置信息
- **SC_EVENT_SEND_ACK_DONE**: SmartConfig完成确认

### 2. 事件处理系统 API (`event_handler.h`)

#### 函数接口

| 函数名 | 说明 | 参数 | 返回值 |
|--------|------|------|--------|
| `event_handler_init()` | 初始化事件处理器 | 无 | 无 |
| `event_handler_register()` | 注册事件处理器 | `event_base`: 事件基础类型<br>`event_id`: 事件ID<br>`event_handler`: 处理函数<br>`event_handler_arg`: 参数 | 无 |
| `event_handler_handle()` | 处理事件 | 事件参数和类型 | 无 |

### 3. I2C驱动 API (`i2c_driver.h`)

#### 函数接口

| 函数名 | 说明 | 参数 | 返回值 |
|--------|------|------|--------|
| `i2c_master_init()` | 初始化I2C控制器 | `i2c_num`: I2C端口号<br>`config`: 配置参数 | `ESP_OK`: 成功<br>`ESP_ERR_INVALID_ARG`: 参数错误 |
| `my_i2c_master_write()` | I2C写入数据 | `i2c_num`: 端口号<br>`dev_addr`: 设备地址<br>`data`: 数据<br>`data_len`: 长度 | `ESP_OK`: 成功 |
| `my_i2c_master_read()` | I2C读取数据 | `i2c_num`: 端口号<br>`dev_addr`: 设备地址<br>`data`: 缓冲区<br>`data_len`: 长度 | `ESP_OK`: 成功 |

#### 配置参数
```c
typedef struct {
    .mode = I2C_MODE_MASTER,          // 主机模式
    .sda_io_num = GPIO_NUM_X,        // SDA引脚
    .scl_io_num = GPIO_NUM_X,        // SCL引脚
    .sda_pullup_en = GPIO_PULLUP_ENABLE, // SDA上拉
    .scl_pullup_en = GPIO_PULLUP_ENABLE, // SCL上拉
    .master.clk_speed = 100000      // 100kHz时钟
} i2c_config_t;
```

### 4. AHT10传感器 API (`aht10.h`)

#### 数据类型
```c
typedef struct {
    float temperature; // 温度 (°C)
    float humidity;    // 湿度 (%)
} aht10_data_t;
```

#### 常量定义
```c
#define AHT10_ADDR 0x38    // I2C地址
#define AHT10_CMD_INIT 0xE1    // 初始化命令
#define AHT10_CMD_MEASURE 0xAC   // 测量命令
#define AHT10_CMD_RESET 0xBA     // 复位命令
```

#### 函数接口

| 函数名 | 说明 | 参数 | 返回值 |
|--------|------|------|--------|
| `aht10_init()` | 初始化AHT10传感器 | `i2c_num`: I2C端口号 | `ESP_OK`: 成功 |
| `aht10_read_data()` | 读取温湿度数据 | `i2c_num`: I2C端口号<br>`data`: 数据结构体指针 | `ESP_OK`: 成功 |

#### 数据格式
- **温度范围**: -40°C 至 85°C
- **湿度范围**: 0% 至 100%
- **精度**: 温度±0.3°C，湿度±2%

### 5. RESTful API 接口

#### 基础信息
- **服务器地址**: `http://esp32.local` (mDNS)
- **IP地址**: 通过SmartConfig获取的局域网IP
- **端口**: 80

#### API端点

| 端点 | 方法 | 说明 | 请求格式 | 响应格式 |
|------|------|------|----------|----------|
| `/api/v1/system/info` | GET | 获取系统信息 | 无 | JSON: `{"version": "v5.4.1", "cores": 2}` |
| `/api/v1/temp/raw` | GET | 获取温度数据 | 无 | JSON: `{"raw": 25.5}` |
| `/api/v1/light/brightness` | POST | 控制灯光亮度 | JSON: `{"red": 255, "green": 128, "blue": 0}` | 文本: "Post control value successfully" |

#### 静态文件服务
- **路径**: `/*` (通配符)
- **功能**: 提供web-demo目录下的静态文件
- **支持的文件类型**: .html, .js, .css, .png, .ico, .svg

### 6. 主程序 API (`main.c`)

#### 函数接口

| 函数名 | 说明 | 参数 | 返回值 |
|--------|------|------|--------|
| `initialise_wifi()` | 初始化WiFi连接 | 无 | 无 |
| `app_main()` | 应用程序入口 | 无 | 无 |

#### 初始化流程
1. **NVS初始化**: `nvs_flash_init()`
2. **事件处理初始化**: `event_handler_init()`
3. **WiFi初始化**: `initialise_wifi()`
4. **SmartConfig初始化**: `smartconfig_init()`
5. **启动SmartConfig任务**: `smartconfig_task()`

## 🔍 使用示例

### 1. 读取AHT10传感器数据
```c
#include "aht10.h"

aht10_data_t sensor_data;
esp_err_t ret = aht10_init(I2C_NUM_0);
if (ret == ESP_OK) {
    ret = aht10_read_data(I2C_NUM_0, &sensor_data);
    printf("温度: %.2f°C, 湿度: %.2f%%\n", sensor_data.temperature, sensor_data.humidity);
}
```

### 2. 使用I2C驱动
```c
#include "i2c_driver.h"

i2c_config_t conf = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = GPIO_NUM_21,
    .scl_io_num = GPIO_NUM_22,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = 100000
};

i2c_master_init(I2C_NUM_0, &conf);
```

### 3. 调用REST API
```bash
# 获取系统信息
curl http://esp32.local/api/v1/system/info

# 控制灯光颜色
curl -X POST http://esp32.local/api/v1/light/brightness \
  -H "Content-Type: application/json" \
  -d '{"red": 255, "green": 128, "blue": 0}'

# 获取温度数据
curl http://esp32.local/api/v1/temp/raw
```

## ⚙️ 配置选项

### 1. 通过menuconfig配置
```bash
idf.py menuconfig
```

#### 配置项
- **WiFi配置**: 设置WiFi模式和参数
- **I2C引脚**: 配置SCL和SDA引脚
- **日志级别**: 设置调试日志级别
- **文件系统**: 选择SPIFFS或SD卡

#### 默认配置
- **I2C SCL**: GPIO_NUM_22
- **I2C SDA**: GPIO_NUM_21
- **AHT10地址**: 0x38
- **WiFi模式**: STA模式

### 2. 编译时配置
在 `sdkconfig.defaults` 文件中预定义配置：
```
CONFIG_ESP32S3_DEFAULT_CPU_FREQ_240=y
CONFIG_ESP32S3_SPIRAM_SUPPORT=y
CONFIG_ESP32S3_FLASH_8MB=y
```

## 🐛 故障排除

### 常见问题

#### 1. 编译错误
```bash
# 清理构建缓存
idf.py fullclean
idf.py build

# 检查依赖
idf.py reconfigure
```

#### 2. WiFi连接失败
- 确认WiFi密码正确
- 检查是否为2.4GHz网络
- 确认信号强度足够

#### 3. I2C通信失败
- 检查SCL/SDA引脚连接
- 确认上拉电阻已启用
- 验证设备地址是否正确

#### 4. 传感器读取失败
- 检查AHT10供电电压(3.3V)
- 确认传感器初始化完成
- 检查I2C总线是否被占用

### 调试信息
```bash
# 启用详细日志
idf.py menuconfig → Component config → Log output → Default log verbosity → Debug

# 查看实时日志
idf.py monitor
```

## 📊 性能指标

### 资源占用
- **Flash占用**: ~0xC4AD0字节 (23%空闲)
- **RAM占用**: ~50KB运行时内存
- **启动时间**: <2秒从复位到WiFi连接

### 功耗
- **WiFi连接**: ~80mA
- **传感器读取**: ~5mA
- **深度睡眠**: ~20μA

## 🔗 相关链接

- **ESP-IDF文档**: https://docs.espressif.com/projects/esp-idf/
- **AHT10数据手册**: https://www.aosong.com/en/products-32.html
- **ESPTOUCH APP**: 
  - Android: [Google Play](https://play.google.com/store/apps/details?id=com.espressif.esptouch)
  - iOS: [App Store](https://apps.apple.com/us/app/espressif-esptouch/id1071179034)

## 📄 许可证

本项目基于MIT许可证开源，详见 [LICENSE](LICENSE) 文件。

## 🤝 贡献指南

欢迎提交Issue和Pull Request！

### 开发环境设置
```bash
# 安装ESP-IDF
./install.sh esp32s3
. ./export.sh

# 构建项目
idf.py build

# 测试
idf.py flash monitor
```
