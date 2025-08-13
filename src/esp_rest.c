/**
 * @file esp_rest.c
 * @brief HTTP RESTful API服务器主程序
 * 
 * 该文件实现了基于ESP-IDF的RESTful API服务器示例，包括网络初始化、
 * mDNS服务配置、文件系统挂载（支持半主机、SD卡和SPIFFS）以及REST服务器启动。
 */
/* HTTP Restful API Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "esp_vfs_semihost.h"
#include "esp_vfs_fat.h"
#include "esp_spiffs.h"
#include "sdmmc_cmd.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"
#include "mdns.h"
#include "lwip/apps/netbiosns.h"
#include "esp_mac.h"
#include "protocol_examples_common.h"
#if CONFIG_EXAMPLE_WEB_DEPLOY_SD
#include "driver/sdmmc_host.h"
#endif

/** mDNS服务实例名称 */
#define MDNS_INSTANCE "esp home web server"

/** 日志标签 */
static const char *TAG = "example";

/**
 * @brief 启动RESTful API服务器
 * @param base_path 网站根目录路径
 * @return ESP_OK表示成功，其他值表示失败
 */
esp_err_t start_rest_server(const char *base_path);

/**
 * @brief 初始化mDNS服务
 * 
 * 设置mDNS主机名、实例名并注册HTTP服务，方便局域网内通过域名访问
 */
static void initialise_mdns(void)
{
    mdns_init();  // 初始化mDNS
    mdns_hostname_set(CONFIG_EXAMPLE_MDNS_HOST_NAME);  // 设置mDNS主机名
    mdns_instance_name_set(MDNS_INSTANCE);  // 设置mDNS实例名

    mdns_txt_item_t serviceTxtData[] = {
        {"board", "esp32"},
        {"path", "/"}
    };

    ESP_ERROR_CHECK(mdns_service_add("ESP32-WebServer", "_http", "_tcp", 80, serviceTxtData,
                                     sizeof(serviceTxtData) / sizeof(serviceTxtData[0])));  // 添加mDNS服务
}

#if CONFIG_EXAMPLE_WEB_DEPLOY_SEMIHOST
/**
 * @brief 初始化半主机文件系统
 * 
 * 注册半主机文件系统用于开发环境下的文件访问
 * @return ESP_OK表示成功，其他值表示失败
 */
esp_err_t init_fs(void)
{
    esp_err_t ret = esp_vfs_semihost_register(CONFIG_EXAMPLE_WEB_MOUNT_POINT);  // 注册半主机文件系统
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register semihost driver (%s)!", esp_err_to_name(ret));
        return ESP_FAIL;
    }
    return ESP_OK;
}
#endif

#if CONFIG_EXAMPLE_WEB_DEPLOY_SD
/**
 * @brief 初始化SD卡文件系统
 * 
 * 配置SD卡硬件接口并挂载FAT文件系统
 * @return ESP_OK表示成功，其他值表示失败
 */
esp_err_t init_fs(void)
{
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    gpio_set_pull_mode(15, GPIO_PULLUP_ONLY); // CMD
    gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);  // D0
    gpio_set_pull_mode(4, GPIO_PULLUP_ONLY);  // D1
    gpio_set_pull_mode(12, GPIO_PULLUP_ONLY); // D2
    gpio_set_pull_mode(13, GPIO_PULLUP_ONLY); // D3

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 4,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_card_t *card;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount(CONFIG_EXAMPLE_WEB_MOUNT_POINT, &host, &slot_config, &mount_config, &card);  // 挂载SD卡文件系统
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }
    /* 打印卡信息 */
    sdmmc_card_print_info(stdout, card);
    return ESP_OK;
}
#endif

#if CONFIG_EXAMPLE_WEB_DEPLOY_SF
/**
 * @brief 初始化SPIFFS文件系统
 * 
 * 注册并挂载SPIFFS文件系统，用于存储网页资源
 * @return ESP_OK表示成功，其他值表示失败
 */
esp_err_t init_fs(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = CONFIG_EXAMPLE_WEB_MOUNT_POINT,
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false
    };
    esp_err_t ret = esp_vfs_spiffs_register(&conf);  // 注册SPIFFS文件系统

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);  // 获取SPIFFS分区信息
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
    return ESP_OK;
}
#endif

/**
 * @brief 应用程序入口函数
 * 
 * 初始化系统组件、网络、文件系统并启动REST服务器
 */
void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());  // 初始化NVS
    ESP_ERROR_CHECK(esp_netif_init());  // 初始化网络接口
    ESP_ERROR_CHECK(esp_event_loop_create_default());  // 创建默认事件循环
    initialise_mdns();  // 初始化mDNS
    netbiosns_init();  // 初始化NetBIOS
    netbiosns_set_name(CONFIG_EXAMPLE_MDNS_HOST_NAME);  // 设置NetBIOS名称

    ESP_ERROR_CHECK(example_connect());  // 连接到网络
    ESP_ERROR_CHECK(init_fs());  // 初始化文件系统
    ESP_ERROR_CHECK(start_rest_server(CONFIG_EXAMPLE_WEB_MOUNT_POINT));  // 启动HTTP服务器
}
