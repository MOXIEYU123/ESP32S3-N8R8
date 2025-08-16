# ESP32S3-N8R8 SmartConfig 项目

基于ESP32-S3的WiFi SmartConfig配置项目，支持通过ESPTOUCH APP进行WiFi配置。

## 项目特性

- **SmartConfig 功能**: 通过ESPTOUCH APP快速配置WiFi连接
- **事件处理系统**: 完整的WiFi事件处理机制
- **传感器支持**: AHT10温湿度传感器数据采集
- **Web服务器**: HTTP REST API接口
- **文件系统**: SPIFFS和FAT文件系统支持
- **I2C通信**: 完整的I2C驱动支持

## 硬件需求

- ESP32-S3开发板 (N8R8 - 8MB Flash, 8MB PSRAM)
- AHT10温湿度传感器模块
- USB数据线

## 软件需求

- ESP-IDF v5.4.1 或更高版本
- ESPTOUCH APP (Android/iOS)

## 快速开始

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

# 烧录到开发板 (替换PORT为你的串口号)
idf.py -p COMx flash

# 监控串口输出
idf.py -p COMx monitor
```

### 4. WiFi配置

1. 确保手机连接到目标WiFi网络 (2.4GHz频段)
2. 打开ESPTOUCH APP
3. 输入WiFi密码
4. 等待配置成功提示

## 项目结构

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
│   └── rest_server.c       # HTTP服务器
├── include/
│   ├── smartconfig.h
│   ├── event_handler.h
│   ├── i2c_driver.h
│   ├── aht10.h
│   └── web_server_handler.h
├── CMakeLists.txt          # 项目配置
└── README.md              # 项目说明
```

## 功能说明

### SmartConfig 流程

1. **启动模式**: 设备启动后进入SmartConfig模式
2. **信道扫描**: 扫描WiFi信道寻找配置包
3. **接收配置**: 接收来自ESPTOUCH APP的配置信息
4. **连接网络**: 使用接收到的SSID和密码连接WiFi
5. **连接成功**: 获取IP地址，SmartConfig完成

### 传感器数据采集

- **AHT10传感器**: 支持温湿度数据采集
- **I2C通信**: 使用标准I2C协议与传感器通信
- **数据格式**: 温度(°C)、湿度(%RH)

### Web服务器功能

- **REST API**: 提供HTTP接口访问传感器数据
- **文件服务**: 支持静态文件服务
- **JSON响应**: 标准化API响应格式

## 配置选项

在 `idf.py menuconfig` 中可以配置：

- WiFi参数
- 传感器引脚配置
- Web服务器端口
- 日志级别

## 故障排除

### 编译问题

- 确保ESP-IDF环境变量已正确设置
- 检查所有依赖组件是否已安装
- 清理构建缓存: `idf.py fullclean`

### 连接问题

- 确认开发板USB驱动已安装
- 检查串口号是否正确
- 确保开发板已进入下载模式

### WiFi配置失败

- 确认手机连接的是2.4GHz WiFi
- 检查WiFi密码是否正确
- 确保ESPTOUCH APP和设备距离足够近

## 技术支持

如有问题，请通过以下方式获取支持：
- 提交GitHub Issue
- 查阅ESP-IDF官方文档
- 访问ESP32中文社区

## 许可证

本项目基于MIT许可证开源。
