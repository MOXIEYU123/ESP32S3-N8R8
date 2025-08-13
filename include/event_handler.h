#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include "esp_event.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_EVENT_HANDLERS 10

void event_handler_init(void);

void event_handler_register(esp_event_base_t event_base, int32_t event_id,
                          esp_event_handler_t event_handler, void* event_handler_arg);

void event_handler_handle(void* arg, esp_event_base_t event_base,
                        int32_t event_id, void* event_data);

#ifdef __cplusplus
}
#endif

#endif // EVENT_HANDLER_H
