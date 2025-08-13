# ESP32-C3 项目指南

## 项目结构
```
.
├── CMakeLists.txt
├── README.md
├── Project_Guide.md
├── sdkconfig
├── sdkconfig.old
├── build/
│   ├── .bin_timestamp
│   ├── .ninja_deps
│   ├── .ninja_log
│   ├── app-flash_args
│   ├── bootloader-flash_args
│   ├── build.ninja
│   ├── cmake_install.cmake
│   ├── CMakeCache.txt
│   ├── compile_commands.json
│   ├── config.env
│   ├── flash_app_args
│   ├── flash_args
│   ├── flash_args.in
│   ├── flasher_args.json
│   ├── kconfigs_projbuild.in
│   ├── kconfigs.in
│   ├── ldgen_libraries
│   ├── ldgen_libraries.in
│   ├── partition-table-flash_args
│   ├── project_description.json
│   ├── smart_config.bin
│   ├── smart_config.elf
│   ├── smart_config.map
│   ├── x509_crt_bundle.S
│   ├── bootloader/
│   ├── config/
│   ├── esp-idf/
│   └── CMakeFiles/
├── github.com/
├── include/
│   ├── event_handler.h
│   └── smartconfig.h
├── main/
│   ├── CMakeLists.txt
│   └── main.c
├── src/
│   ├── code_summary.md
│   ├── esp_rest.c
│   ├── event_handler.c
│   ├── integration_guide.md
│   ├── rest_server.c
│   └── smartconfig.c
└── web-demo/
    ├── .browserslistrc
    ├── .editorconfig
    ├── .eslintrc.js
    ├── .gitignore
    ├── babel.config.js
    ├── package.json
    ├── postcss.config.js
    ├── vue.config.js
    └── public/
    └── src/
```

## 关键文件说明
1. **CMakeLists.txt** - 项目构建配置文件（主目录）
2. **main/main.c** - 主程序入口文件
3. **src/esp_rest.c** - REST API实现核心代码
4. **src/smartconfig.c** - 智能配置功能实现
5. **web-demo/** - 前端演示项目目录
6. **include/event_handler.h** - 事件处理模块头文件

## 一、开发环境搭建（Windows系统）
### 1.1 安装ESP-IDF
1. 访问[乐鑫官网](https://www.espressif.com/)下载ESP-IDF工具链（选择v5.1以上版本）
2. 运行安装程序，保持默认路径（建议不要包含中文）
3. 安装完成后，打开“ESP-IDF Command Prompt”验证环境：
   ```bash
   idf.py --version
   ```
   - 成功标志：显示版本号（如v5.1.2）

### 1.2 配置VS Code
1. 安装VS Code（[下载地址](https://code.visualstudio.com/)）
2. 安装ESP-IDF插件：
   - 打开扩展商店，搜索“Espressif IDF”并安装
3. 配置插件路径：
   - 按`Ctrl+Shift+P`打开命令面板，输入“ESP-IDF: Configure Paths”
   - 选择ESP-IDF安装目录（如`C:\Espressif\frameworks\esp-idf-v5.1`）

## 二、硬件准备
### 2.1 开发板选择
推荐使用**ESP32-C3-DevKitC-02**（兼容本项目）
- 核心参数：
  - 芯片：ESP32-C3（RISC-V 32位处理器）
  - 无线：2.4GHz Wi-Fi 4 + Bluetooth 5 (LE)
  - 接口：USB-C（供电/调试）

### 2.2 接线说明
- 无需额外接线（所有功能通过板载接口实现）
- 注意：首次使用需连接USB线到电脑（建议使用原装线）

## 三、项目结构详解
```
.
├── main/           # 主程序目录
│   ├── main.c      # 程序入口（包含app_main函数）
│   └── CMakeLists.txt  # 组件构建配置
├── src/            # 功能模块源码
│   ├── smartconfig.c  # 智能配网核心代码
│   ├── event_handler.c  # 事件处理逻辑
│   └── esp_rest.c    # REST API实现
├── include/        # 头文件目录
│   ├── smartconfig.h  # 智能配网接口声明
│   └── event_handler.h  # 事件处理接口声明
├── web-demo/       # 前端演示项目（可选）
└── build/          # 构建产物目录（编译后生成）
```

### 3.1 系统架构设计
采用事件驱动架构，核心由两大模块组成：
- **事件处理器模块(event_handler)**：负责事件注册、分发的通用管理框架
- **智能配网模块(smartconfig)**：实现ESP-TOUCH配网协议的核心逻辑

### 3.2 核心模块功能
#### 3.2.1 事件处理器模块（src/event_handler.c）
作为通用事件管理框架，主要提供三大功能：
- 初始化：`event_handler_init()` 初始化处理器数组并重置计数器
- 注册：`event_handler_register()` 调用ESP-IDF的`esp_event_handler_register`注册新处理器（最多5个）
- 处理：`event_handler_handle()` 遍历所有注册处理器并调用其处理函数

与SmartConfig模块关系：SmartConfig通过本模块注册专属事件处理函数，事件触发时由本模块调用执行

#### 3.2.2 智能配网模块（src/smartconfig.c）
实现配网核心逻辑，包含三大功能单元：
- 初始化：`smartconfig_init()` 创建事件组、初始化事件处理器并注册配网事件
- 事件处理：`smartconfig_event_handler()` 处理`SC_EVENT_GOT_SSID_PSWD`（获取WiFi信息）和`SC_EVENT_SEND_ACK_DONE`（确认发送完成）事件
- 任务管理：`smartconfig_task()` 启动配网任务、等待连接完成后停止并清理资源
```

## 四、关键代码解析（以SmartConfig为例）
### 4.1 事件处理函数（src/smartconfig.c）
```c
static void smartconfig_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {  // 收到WiFi信息事件
        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;  // 定义WiFi配置结构体

        // 从事件数据中提取SSID和密码
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));

        ESP_ERROR_CHECK(esp_wifi_disconnect());   // 断开当前连接（如果有）
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));  // 设置新配置
        ESP_ERROR_CHECK(esp_wifi_connect());    // 连接目标WiFi
    }
}
```
- 关键步骤说明：
  1. **事件过滤**：仅处理`SC_EVENT_GOT_SSID_PSWD`事件（收到手机发送的WiFi信息）
  2. **数据提取**：从事件数据中读取SSID（WiFi名称）和密码
  3. **连接WiFi**：通过`esp_wifi_connect()`函数连接目标网络

### 4.2 主函数调用（main/main.c）
```c
void app_main(void)
{
    // 初始化事件循环（必须步骤）
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // 注册SmartConfig事件处理函数
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, 
                                                &smartconfig_event_handler, NULL));

    // 启动SmartConfig配网
    smartconfig_start();
}
```
- 代码作用：
  - `esp_event_loop_create_default()`：创建默认事件循环（用于处理系统事件）
  - `esp_event_handler_register()`：将`smartconfig_event_handler`注册为SmartConfig事件的处理函数
  - `smartconfig_start()`：启动配网流程（此时开发板会进入等待配网状态）

### 4.3 关键数据结构说明
根据系统设计，涉及以下核心数据结构：
- `EventGroupHandle_t`：FreeRTOS事件组句柄，用于任务间同步（如配网完成标志）
- `wifi_config_t`：ESP-IDF定义的WiFi配置结构体，包含`sta.ssid`（SSID字段）和`sta.password`（密码字段）
- `smartconfig_event_got_ssid_pswd_t`：SmartConfig事件数据结构体，包含手机发送的SSID、密码及认证类型等信息

### 4.4 系统主要运行流程
结合代码实现，完整配网流程可分解为6个关键步骤：
1. **系统初始化**：通过`app_main()`完成事件循环创建和模块初始化
2. **事件注册**：调用`esp_event_handler_register()`注册SmartConfig事件处理函数
3. **启动配网**：执行`smartconfig_start()`启动配网任务（进入等待手机发送信息状态）
4. **接收数据**：触发`SC_EVENT_GOT_SSID_PSWD`事件时，从事件数据提取WiFi信息
5. **连接网络**：通过`esp_wifi_set_config()`设置参数，调用`esp_wifi_connect()`连接目标WiFi
6. **清理资源**：配网完成后通过`smartconfig_task()`停止配网任务并释放资源

## 五、构建与烧录（超详细步骤）
### 5.1 配置串口
1. 打开设备管理器，查看开发板对应的串口（如COM15）
2. 替换命令中的`COM15`为实际串口号

### 5.2 构建项目
```bash
idf.py -p COM15 build
```
- 首次构建需下载依赖（约5-10分钟）
- 成功标志：终端显示`Project build complete`
- 构建产物路径：`build/smart_config.bin`（可烧录固件）

### 5.3 烧录固件
```bash
idf.py -p COM15 flash
```
- 烧录时开发板会自动重启
- 成功标志：终端显示`Hash of data verified`

### 5.4 查看运行日志
```bash
idf.py -p COM15 monitor
```
- 按`Ctrl+]`退出监控
- 正常日志示例：
  ```
  I (123) smartconfig: 获取到 SSID: MyWiFi
  I (125) smartconfig: 密码: 12345678
  I (234) wifi: connected to ap SSID=MyWiFi
  ```

## 六、功能验证
### 6.1 智能配网测试
1. 手机下载`EspTouch`工具（应用商店搜索）
2. 输入当前WiFi名称和密码，点击开始配网
3. 观察开发板STATUS灯：
   - 快闪：等待配网
   - 慢闪：连接WiFi中
   - 常亮：配网成功

### 6.2 常见问题排查
| 问题现象 | 可能原因 | 解决方法 |
|----------|----------|----------|
| 手机搜不到开发板 | 蓝牙未开启 | 打开手机蓝牙（EspTouch需蓝牙辅助） |
| 配网超时 | WiFi为5GHz频段 | 切换为2.4GHz频段（ESP32-C3不支持5G） |
| 连接失败 | 密码错误 | 检查手机输入的WiFi密码是否正确 |

## 注意事项
- 确保COM15是实际连接的串口端口
- 如果遇到`idf.py`命令未识别错误，请检查ESP-IDF环境变量配置
- 构建产物位于`build/`目录
- 智能配置功能需要配合`include/smartconfig.h`使用
