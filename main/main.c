/* Esptouch example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_eap_client.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "smartconfig.h"
#include "event_handler.h" // 包含新的头文件
#include "web_server_handler.h"

static const char *TAG = "main";

// 初始化WiFi
static void initialise_wifi(void)
{
    ESP_ERROR_CHECK(esp_netif_init()); // 初始化网络接口
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // 创建默认事件循环
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta(); // 创建默认的WiFi STA接口
    assert(sta_netif);

/* Register event handlers */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler_handle, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler_handle, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler_handle, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // 获取默认的WiFi初始化配置
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) ); // 初始化WiFi

    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) ); // 设置WiFi模式为STA
    ESP_ERROR_CHECK( esp_wifi_start() ); // 启动WiFi
}

// 应用程序入口
void app_main(void)
{

    ESP_ERROR_CHECK( nvs_flash_init() ); // 初始化NVS

    /* Initialize event handler */
    event_handler_init();

    initialise_wifi(); // 初始化WiFi

    /* Initialize smartconfig */
    smartconfig_init(); // 初始化smartconfig

    smartconfig_task(NULL); // 启动smartconfig任务


}
