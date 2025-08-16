<<<<<<< HEAD
idf_component_register(SRCS "main.c" "../src/smartconfig.c" "../src/event_handler.c" "../src/i2c_driver.c" "../src/aht10.c"
                    PRIV_REQUIRES spi_flash esp_wifi esp_netif nvs_flash esp_event wpa_supplicant esp_http_server vfs json driver fatfs spiffs
                    INCLUDE_DIRS "." "../include")
