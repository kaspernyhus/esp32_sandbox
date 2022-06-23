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
#include "cJSON.h"
#include <sys/param.h>

char ip_addr[20];



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

void set_ip(char* ip)
{
    memcpy(ip_addr,ip,13);
    ESP_LOGI("ip","%s",ip_addr);
}


esp_err_t set_ip_handler(httpd_req_t *req)
{
    char content[100];

    /* Truncate if content length larger than the buffer */
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {  /* 0 return value indicates connection closed */
        /* Check if timeout occurred */
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            /* In case of timeout one can choose to retry calling
             * httpd_req_recv(), but to keep it simple, here we
             * respond with an HTTP 408 (Request Timeout) error */
            httpd_resp_send_408(req);
        }
        /* In case of error, returning ESP_FAIL will
         * ensure that the underlying socket is closed */
        return ESP_FAIL;
    }
    
    cJSON *root = cJSON_Parse(content);
    char* new_ip_addr = cJSON_GetObjectItem(root, "ip")->valuestring;

    // ESP_LOGI("POST","ip = %s", new_ip_addr);
    set_ip(new_ip_addr);

    cJSON_Delete(root);

    httpd_resp_sendstr(req, "Post control value successfully\n");
    
    return ESP_OK;
}






void app_main(void)
{
     /******* WiFi ********/
    ESP_ERROR_CHECK(nvs_flash_init());   // Initialize nvs flash to get wifi config
    wifi_init_sta();                     // Start wifi in station mode


    start_webserver();
    swaggerui_init();

    webserver_register_uri("/hej", HTTP_GET, hej_handler);
    webserver_register_uri("/set_ip", HTTP_POST, set_ip_handler);

    char* adr = "123.456.1.34"; 
    set_ip(adr);
}

