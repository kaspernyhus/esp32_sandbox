#include "test_module.h"
#include "esp_log.h"
#include "string.h"

static char ip_addr[] = "123.456.789.101";

esp_err_t setIP(char *ip)
{
    strcpy(ip_addr,ip);
    ESP_LOGI("test module", "setIP");
    return ESP_OK;
}

esp_err_t getIP(char *ip)
{
    strcpy(ip,ip_addr);
    ESP_LOGI("test module", "getIP");
    return ESP_OK;
}

esp_err_t setModel(models_t model)
{
    ESP_LOGI("test module", "setModel");
    return ESP_OK;
}

esp_err_t getModel(models_t *model)
{
    ESP_LOGI("test module", "getModel");
    return ESP_OK;
}

esp_err_t command(commands_t command)
{
    ESP_LOGI("test module", "command");
    return ESP_OK;
}
