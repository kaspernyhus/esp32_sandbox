#include "web_server.h"
#include "esp_log.h"
#include <sys/param.h>

#include "esp_chip_info.h"
#include "esp_spi_flash.h"
#include "cJSON.h"


static const char *WEB_SERVER_TAG = "web server";
httpd_handle_t server = NULL;

static esp_err_t system_info_get_handler(httpd_req_t *req)
{
    char buf[1024];
    char  host[16];
    size_t hdr_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (hdr_len > 1) {
        httpd_req_get_hdr_value_str(req, "Host", host, hdr_len);
    }

    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    char* chip_type;
    esp_chip_model_t model = chip_info.model;
    switch (model) {
        case CHIP_ESP32:
            chip_type = "ESP32";
            break;
        case CHIP_ESP32S2:
            chip_type = "ESP32S2";
            break;
        case CHIP_ESP32S3:
            chip_type = "ESP32S3";
            break;
        case CHIP_ESP32C3:
            chip_type = "ESP32C3";
            break;
        case CHIP_ESP32H2:
            chip_type = "ESP32H2";
            break;
        default:
            chip_type = "unknown";
            break;
    }

    cJSON *root;
    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "Chip", chip_type);
    cJSON_AddNumberToObject(root, "Silicon revision", chip_info.revision);
    cJSON_AddNumberToObject(root, "Number of cores", chip_info.cores);
    sprintf(buf,"%uMB %s",spi_flash_get_chip_size() / (1024 * 1024),(chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    cJSON_AddStringToObject(root, "Flash size", buf);
    sprintf(buf,"%s%s%s%s%s%s", 
                    (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "Embedded flash memory" : "", 
                    (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "/2.4GHz WiFi" : "", 
                    (chip_info.features & CHIP_FEATURE_BLE) ? "/Bluetooth LE" : "",
                    (chip_info.features & CHIP_FEATURE_BT) ? "/Bluetooth Classic" : "",
                    (chip_info.features & CHIP_FEATURE_IEEE802154) ? "/IEEE 802.15.4" : "",
                    (chip_info.features & CHIP_FEATURE_EMB_PSRAM) ? "/Embedded PSRAM" : "");
    cJSON_AddStringToObject(root, "Features", buf);

    char *json_str;
    json_str = cJSON_Print(root);
    
    httpd_resp_send(req, json_str, HTTPD_RESP_USE_STRLEN);
    // ESP_LOGI(WEB_SERVER_TAG, "%s", json_str);
    ESP_LOGI(WEB_SERVER_TAG, "Response send to %s", host);
    
    free(json_str);
    cJSON_Delete(root);

    return ESP_OK;
}

static const httpd_uri_t system_stats = {
    .uri       = "/api/system/info",
    .method    = HTTP_GET,
    .handler   = system_info_get_handler,
    .user_ctx  = NULL
};

esp_err_t webserver_register_uri(char* uri, httpd_method_t method, esp_err_t (*handler)(httpd_req_t *r))
{
    httpd_uri_t new_uri = {
        .uri = uri,
        .method = method,
        .handler = handler
    };
    httpd_register_uri_handler(server, &new_uri);
    return ESP_OK;
}

esp_err_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(WEB_SERVER_TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(WEB_SERVER_TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &system_stats);



        return ESP_OK;
    }

    ESP_LOGE(WEB_SERVER_TAG, "Error starting web server!");
    return ESP_FAIL;
}

void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
    ESP_LOGI(WEB_SERVER_TAG, "Stopped");
}

