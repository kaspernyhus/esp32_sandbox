/* 

*/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_err.h"

#include "wifi_config.h"
#include "web_server.h"
#include "swaggerui.h"

#include "test_module.h"


void app_main(void)
{
     /******* WiFi ********/
    ESP_ERROR_CHECK(nvs_flash_init());   // Initialize nvs flash to get wifi config
    wifi_init_sta();                     // Start wifi in station mode


    start_webserver();
    swaggerui_init();
}

