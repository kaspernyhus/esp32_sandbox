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


void hej()
{
    ESP_LOGI("hej","hest");
}


esp_err_t hej_handler(httpd_req_t *req)
{
    hej();
    httpd_resp_send(req, "hej hej hest", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}


void app_main(void)
{
     /******* WiFi ********/
    ESP_ERROR_CHECK(nvs_flash_init());   // Initialize nvs flash to get wifi config
    wifi_init_sta();                     // Start wifi in station mode


    start_webserver();

    webserver_register_uri("/hej", HTTP_GET, hej_handler);
}

