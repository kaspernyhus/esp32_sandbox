/*
 * WiFi Real Time Logging
*/

#include <stdio.h>
#include "wifi_config.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_heap_task_info.h"
#include "esp_heap_caps.h"

#include "remote_log.h"


#define LOGGING_INTERVAL_MS 10


int8_t heap_logger(uint8_t *out, size_t *len);


void app_main(void)
{
    /******* WiFi ********/
    // Initialize nvs flash to get wifi config
    ESP_ERROR_CHECK(nvs_flash_init());
    // Start wifi in station mode
    wifi_init_sta();
    // Wait for connection...
    vTaskDelay(1000/portTICK_PERIOD_MS);


    remote_log_init(LOGGING_INTERVAL_MS, "192.168.0.53", 3333);
    

    remote_log_id_t logger_id = {
        .log_id = 0x12,
        .tag = "Heap Logger",
        .tag_len = 12
    };
    remote_log_register_t logger = {
        .id = logger_id,
        .cb = heap_logger
    };
    
    remote_log_register(logger);
}


int8_t heap_logger(uint8_t *out, size_t *len)
{
    uint32_t val = esp_get_free_heap_size();
    memcpy(out,(uint8_t*)&val,4);
    *len = 4;

    return 0;
}
