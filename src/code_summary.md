### 事件管理框架的工作原理

1. **事件管理框架的初始化**：
   - `event_handler_init` 函数用于初始化事件处理器数组，并重置处理器计数器。

   ```c
   void event_handler_init(void)
   {
       for (int i = 0; i < MAX_EVENT_HANDLERS; i++) {
           event_handlers[i] = NULL;
       }
       handler_count = 0;
   }
   ```

2. **注册事件处理函数**：
   - `event_handler_register` 函数用于注册新的事件处理函数。它会将处理函数存储在 `event_handlers` 数组中，并调用 `esp_event_handler_register` 注册到ESP-IDF的事件系统中。

   ```c
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
   ```

3. **处理事件**：
   - 当事件发生时，ESP-IDF会调用注册的事件处理函数。`event_handler_handle` 函数会遍历所有注册的事件处理函数，并调用它们来处理事件。

   ```c
   void event_handler_handle(void* arg, esp_event_base_t event_base,
                           int32_t event_id, void* event_data)
   {
       for (int i = 0; i < handler_count; i++) {
           if (event_handlers[i] != NULL) {
               event_handlers[i](arg, event_base, event_id, event_data);
           }
       }
   }
   ```

4. **SmartConfig模块的初始化**：
   - `smartconfig_init` 函数会初始化事件处理器模块，并注册SmartConfig的事件处理函数。

   ```c
   void smartconfig_init(void)
   {
       s_wifi_event_group = xEventGroupCreate();
       
       event_handler_init();
       
       event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &smartconfig_event_handler, NULL);
   }
   ```

5. **SmartConfig事件处理函数**：
   - `smartconfig_event_handler` 函数用于处理SmartConfig特定的事件，例如获取SSID和密码。

   ```c
   static void smartconfig_event_handler(void* arg, esp_event_base_t event_base,
                                       int32_t event_id, void* event_data)
   {
       if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
           ESP_LOGI(TAG, "获取到 SSID 和密码");

           smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
           wifi_config_t wifi_config;
           uint8_t ssid[33] = { 0 };
           uint8_t password[65] = { 0 };

           memset(&wifi_config, 0, sizeof(wifi_config_t));
           memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
           memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));

           ESP_LOGI(TAG, "SSID:%s", ssid);
       }
   }
   ```

### 添加新功能模块的步骤

1. **定义事件处理函数**：
   - 每个新模块需要定义自己的事件处理函数，用于处理特定的事件。

   ```c
   static void new_module_event_handler(void* arg, esp_event_base_t event_base,
                                        int32_t event_id, void* event_data)
   {
       if (event_base == NEW_EVENT && event_id == NEW_EVENT_ID) {
           ESP_LOGI(TAG, "处理新的事件");
           // 处理事件的具体逻辑
       }
   }
   ```

2. **包含 `event_handler.h` 头文件**：
   - 在新模块的源文件中包含 `event_handler.h` 头文件，以便使用事件管理框架的功能。

   ```c
   #include "event_handler.h"
   #include "esp_log.h"
   ```

3. **注册事件处理函数**：
   - 在新模块的初始化函数中，通过 `event_handler_register` 函数将事件处理函数注册到事件管理框架中。

   ```c
   void new_module_init(void)
   {
       /* 注册 new_module 事件处理程序 */
       event_handler_register(NEW_EVENT, NEW_EVENT_ID, &new_module_event_handler, NULL);
   }
   ```

通过这种方式，新的功能模块可以方便地集成到现有的事件管理框架中，实现统一的事件处理逻辑。这样不仅提高了代码的可维护性和可扩展性，还使得各个模块之间的耦合度降低，便于独立开发和测试。
