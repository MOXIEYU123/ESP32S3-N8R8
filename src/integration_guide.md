# SmartConfig与REST API集成指南

## 概述
本指南介绍如何在SmartConfig配网成功后，集成RESTful API服务器功能。该功能包括：
- 提供Web管理界面
- 系统信息查询
- 温度数据采集
- 灯光控制

## 集成步骤

### 1. 添加头文件引用
在main.c中添加以下头文件引用：
```c
#include "smartconfig.h"
#include "web_server_handler.h"
#include "esp_event.h"
```

### 2. 初始化事件处理
在app_main()中添加以下初始化代码：
```c
// 初始化事件循环
ESP_ERROR_CHECK(esp_event_loop_create_default());

// 注册SmartConfig事件处理
ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &smartconfig_event_handler, NULL));

// 启动SmartConfig
smartconfig_start();
```

### 3. 事件处理函数实现
实现SmartConfig事件处理函数：

```c
static void smartconfig_event_handler(void* arg, esp_event_base_t event_base, 
                                    int32_t event_id, void* event_data)
{
    if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        // 原有配网成功处理逻辑...

        // 新增：启动REST API服务器
        ESP_ERROR_CHECK(start_rest_server(CONFIG_WEB_MOUNT_POINT));
        ESP_LOGI(TAG, "REST API server started");
    }
}
```

### 2. 配置Kconfig选项
在`Kconfig.projbuild`中添加以下配置选项：

```kconfig
config WEB_MOUNT_POINT
    string "Web server mount point"
    default "/spiffs"
    help
        Mount point for web server files

config EXAMPLE_MDNS_HOST_NAME
    string "mDNS Host Name"
    default "esp32"
    help
        mDNS host name for device
```

### 3. 文件系统初始化
在`app_main()`中添加文件系统初始化代码：

```c
void app_main(void)
{
    // 原有初始化代码...

    // 初始化文件系统
    ESP_ERROR_CHECK(init_fs());

    // 初始化mDNS
    initialise_mdns();
}
```

### 4. API接口说明

#### 4.1 系统信息
- URL: `/api/v1/system/info`
- Method: GET
- 返回格式: JSON
- 示例：
```json
{
    "version": "v5.4",
    "cores": 2
}
```

#### 4.2 温度数据
- URL: `/api/v1/temp/raw`
- Method: GET
- 返回格式: JSON
- 示例：
```json
{
    "raw": 25
}
```

#### 4.3 灯光控制
- URL: `/api/v1/light/brightness`
- Method: POST
- 请求格式: JSON
- 示例请求：
```json
{
    "red": 255,
    "green": 128,
    "blue": 0
}
```

### 5. Web管理界面部署

#### 5.1 构建Vue项目
1. 进入web-demo目录
```bash
cd D:\Espressif\v5.4\smart_config\web-demo
```

2. 安装依赖
```bash
npm install
```

3. 构建生产环境文件
```bash
npm run build
```

构建完成后，生成的文件将位于`dist`目录下。

#### 5.2 部署到ESP32-C3

1. 配置SPIFFS分区
在`partitions.csv`中添加以下内容：
```
# Name,   Type, SubType, Offset,  Size, Flags
web,      data, spiffs,  ,        1M,
```

2. 修改Kconfig配置
在`Kconfig.projbuild`中添加：
```kconfig
config WEB_MOUNT_POINT
    string "Web server mount point"
    default "/spiffs"
    help
        Mount point for web server files

config WEB_PARTITION_LABEL
    string "Web partition label"
    default "web"
    help
        Partition label for web files
```

3. 初始化SPIFFS文件系统
在`app_main()`中添加：
```c
esp_vfs_spiffs_conf_t conf = {
    .base_path = CONFIG_WEB_MOUNT_POINT,
    .partition_label = CONFIG_WEB_PARTITION_LABEL,
    .max_files = 5,
    .format_if_mount_failed = true
};
ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));
```

4. 上传文件到ESP32-C3
使用以下命令上传构建好的web文件：
```bash
python $IDF_PATH/components/esptool_py/esptool/esptool.py --chip esp32c3 \
    --port COMX --baud 921600 --before default_reset --after hard_reset write_flash \
    0x110000 web-demo/dist/*
```

5. 验证文件系统
在代码中添加文件系统验证：
```c
size_t total = 0, used = 0;
esp_spiffs_info(CONFIG_WEB_PARTITION_LABEL, &total, &used);
ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
```

#### 5.3 访问Web界面
通过浏览器访问设备IP地址即可使用Web管理界面，主要功能包括：
- 系统状态监控
- 温度数据图表
- 灯光控制面板

## 注意事项
1. 确保文件系统已正确配置并包含Web界面文件
2. mDNS服务需要与路由器在同一局域网
3. 温度数据为示例数据，需根据实际传感器修改
4. 灯光控制接口需根据实际硬件实现
