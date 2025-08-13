#include "smartconfig.h"
#include "event_handler.h"
#include "esp_log.h"
#include "freertos/task.h"
#include <string.h>
#include "esp_mac.h"
#include "sdkconfig.h"

static const char *TAG = "smartconfig";

/* 用于指示我们已连接并准备好发出请求的事件组 */
EventGroupHandle_t s_wifi_event_group;

/* 声明 smartconfig_event_handler 函数 */
static void smartconfig_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data);

void smartconfig_init(void)
{
    s_wifi_event_group = xEventGroupCreate();   // 创建事件组
    
    /* 注册 smartconfig 事件处理程序 */
    event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &smartconfig_event_handler, NULL);
}

static void smartconfig_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    /* 处理 smartconfig 特定事件 */
    if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        ESP_LOGI(TAG, "获取到 SSID 和密码");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;
        uint8_t ssid[33] = { 0 };
        uint8_t password[65] = { 0 };
        uint8_t rvd_data[33] = { 0 };

        memset(&wifi_config, 0, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));

#ifdef CONFIG_SET_MAC_ADDRESS_OF_TARGET_AP
        wifi_config.sta.bssid_set = evt->bssid_set;
        if (wifi_config.sta.bssid_set == true) {
            ESP_LOGI(TAG, "设置目标 AP 的 MAC 地址: "MACSTR" ", MAC2STR(evt->bssid));
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
        }
#endif

        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));
        ESP_LOGI(TAG, "SSID:%s", ssid);
        ESP_LOGI(TAG, "密码:%s", password);
        if (evt->type == SC_TYPE_ESPTOUCH_V2) {
            ESP_ERROR_CHECK( esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)) );
            ESP_LOGI(TAG, "RVD 数据:");
            for (int i=0; i<33; i++) {
                printf("%02x ", rvd_data[i]);
            }
            printf("\n");
        }

        ESP_ERROR_CHECK( esp_wifi_disconnect() );   // 断开当前连接
        ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );  // 设置 WiFi 配置
        ESP_ERROR_CHECK(esp_wifi_connect());    // 连接到 AP
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }
}



/**
 * @brief 智能配置任务函数
 * 
 * 本函数负责初始化智能配置功能，并等待智能配置完成或Wi-Fi连接成功。
 * 它使用了ESP-IDF的智能配置API来简化设备的Wi-Fi配置过程。
 * 
 * @param parm 任务参数，本函数中未使用
 */
void smartconfig_task(void * parm)
{
    EventBits_t uxBits;

    // 设置智能配置类型为ESPTouch
    ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH) );

    // 初始化智能配置启动配置
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();

    // 启动智能配置
    ESP_ERROR_CHECK( esp_smartconfig_start(&cfg) );

    while (1) {
        // 等待Wi-Fi连接成功或智能配置完成的事件
        uxBits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);

        // 检查是否已连接到Wi-Fi
        if(uxBits & CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi 已连接到 AP");
        }

        // 检查智能配置是否完成
        if(uxBits & ESPTOUCH_DONE_BIT) {
            ESP_LOGI(TAG, "smartconfig 完成");

            // 停止智能配置
            esp_smartconfig_stop();

            // 删除当前任务
            vTaskDelete(NULL);
        }
    }
}