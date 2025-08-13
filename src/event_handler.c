/**
 * @file event_handler.c
 * @brief 事件处理器模块实现
 * 
 * 该模块提供事件处理器的注册、初始化和事件分发功能，
 * 支持最多注册MAX_EVENT_HANDLERS个事件处理函数。
 */
#include "event_handler.h"
#include "esp_log.h"
#include "freertos/task.h"
#include <string.h>
#include "esp_mac.h"
#include "sdkconfig.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_smartconfig.h"

#define MAX_EVENT_HANDLERS 10

static const char *TAG = "event_handler";

/** 已注册的事件处理器数组 */
static esp_event_handler_t event_handlers[MAX_EVENT_HANDLERS];      
/** 当前已注册的事件处理器数量 */
static int handler_count = 0;

/**
 * @brief 初始化事件处理器模块
 * 
 * 重置事件处理器数组和计数器，将所有处理器指针设为NULL
 */
void event_handler_init(void)
{
    /* Initialize event handlers array */
    for (int i = 0; i < MAX_EVENT_HANDLERS; i++) {
        event_handlers[i] = NULL;
    }
    handler_count = 0;
}

/**
 * @brief 注册新的事件处理器
 * 
 * @param event_base 事件基础类型
 * @param event_id 要处理的事件ID
 * @param event_handler 事件处理函数指针
 * @param event_handler_arg 传递给事件处理函数的参数
 * 
 * @note 如果已达到最大处理器数量，将记录错误日志
 */
void event_handler_register(esp_event_base_t event_base, int32_t event_id,
                          esp_event_handler_t event_handler, void* event_handler_arg)
{
    if (handler_count < MAX_EVENT_HANDLERS) {
        event_handlers[handler_count++] = event_handler;
        ESP_ERROR_CHECK(esp_event_handler_register(event_base, event_id, event_handler, event_handler_arg));
    } else {
        ESP_LOGE(TAG, "Cannot register more event handlers, maximum reached");
    }
}

/**
 * @brief 处理事件并分发给所有注册的处理器
 * 
 * @param arg 事件参数
 * @param event_base 事件基础类型
 * @param event_id 事件ID
 * @param event_data 事件数据指针
 * 
 * 遍历所有已注册的事件处理器并调用它们
 */
void event_handler_handle(void* arg, esp_event_base_t event_base,
                        int32_t event_id, void* event_data)
{
    /* Call all registered event handlers */
    for (int i = 0; i < handler_count; i++) {
        if (event_handlers[i] != NULL) {
            event_handlers[i](arg, event_base, event_id, event_data);
        }
    }
}
