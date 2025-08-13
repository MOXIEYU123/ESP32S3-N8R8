#ifndef __SMARTCONFIG_H__
#define __SMARTCONFIG_H__

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_smartconfig.h"
#include "freertos/event_groups.h"

/* Event group to signal when we are connected & ready to make a request */
extern EventGroupHandle_t s_wifi_event_group;

/* Event bits */
#define CONNECTED_BIT BIT0
#define ESPTOUCH_DONE_BIT BIT1

/* Event handler callback type */
typedef void (*smartconfig_event_cb_t)(esp_event_base_t event_base, 
                                     int32_t event_id, 
                                     void* event_data);

/* Function declarations */
void smartconfig_init(void);
void smartconfig_register_event_handler(smartconfig_event_cb_t cb);
void smartconfig_task(void * parm);

#endif /* __SMARTCONFIG_H__ */
