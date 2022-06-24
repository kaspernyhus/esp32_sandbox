#include "web_server.h"
#include "esp_log.h"
#include <sys/param.h>

#include "esp_chip_info.h"
#include "esp_spi_flash.h"
#include "cJSON.h"

#include "test_module.h"


static const char *WEB_SERVER_TAG = "web server";
httpd_handle_t server = NULL;


esp_err_t ml_model_set_handler(httpd_req_t *req)
{
    char content[25];
    size_t recv_size = 25;

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }

    if(strcmp(content,"Beosound3200") == 0) {
        setModel(BS3200);
    }
    else if(strcmp(content,"Beosound9000") == 0) {
        setModel(BS9000);
    }
    else {
        httpd_resp_send_err(req, HTTPD_405_METHOD_NOT_ALLOWED, "Invalid masterlink product");
        return ESP_FAIL;
    }

    httpd_resp_sendstr(req, "Masterlink product selected\n");
    return ESP_OK;
}

static const httpd_uri_t set_ml_model = {
    .uri       = "/api/v1/masterlink/model",
    .method    = HTTP_POST,
    .handler   = ml_model_set_handler,
    .user_ctx  = NULL
};

esp_err_t ml_model_get_handler(httpd_req_t *req)
{
    cJSON *root;
    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "model", "BeoSound9000");

    char *json_str;
    json_str = cJSON_Print(root);
    
    httpd_resp_send(req, json_str, HTTPD_RESP_USE_STRLEN);
    
    free(json_str);
    cJSON_Delete(root);
    return ESP_OK;
}

static const httpd_uri_t get_ml_model = {
    .uri       = "/api/v1/masterlink/model",
    .method    = HTTP_GET,
    .handler   = ml_model_get_handler,
    .user_ctx  = NULL
};

esp_err_t ip_set_handler(httpd_req_t *req)
{
    char content[16];

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
    
    ESP_LOGI("set ip", "%s", content);

    setIP(content);

    // cJSON *root = cJSON_Parse(content);
    // char* new_ip_addr = cJSON_GetObjectItem(root, "ip")->valuestring;

    // // ESP_LOGI("POST","ip = %s", new_ip_addr);
    // setIP(new_ip_addr);

    // cJSON_Delete(root);

    httpd_resp_sendstr(req, "Post control value successfully\n");
    
    return ESP_OK;
}

static const httpd_uri_t set_ip = {
    .uri       = "/api/v1/streaming/ip",
    .method    = HTTP_POST,
    .handler   = ip_set_handler,
    .user_ctx  = NULL
};

esp_err_t ip_get_handler(httpd_req_t *req)
{
    char ip[16];
    getIP(ip);
    ESP_LOGI("ip","%s", ip);

    httpd_resp_sendstr(req, ip);
    return ESP_OK;
}

static const httpd_uri_t get_ip = {
    .uri       = "/api/v1/streaming/ip",
    .method    = HTTP_GET,
    .handler   = ip_get_handler,
    .user_ctx  = NULL
};

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
    .uri       = "/api/v1/system/info",
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
        httpd_register_uri_handler(server, &set_ip);
        httpd_register_uri_handler(server, &get_ip);
        httpd_register_uri_handler(server, &set_ml_model);
        httpd_register_uri_handler(server, &get_ml_model);

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

