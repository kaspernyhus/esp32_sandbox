/*
 * WiFi Real Time Logging
*/

#include <stdio.h>
#include "wifi_config.h"
#include "nvs_flash.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "circular_buffer.h"
#include "code_timer.h"
#include "log_buffer.h"
#include "tcp_stream.h"
#include "udp_stream.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_heap_task_info.h"
#include "esp_heap_caps.h"



#define LOGGING_INTERVAL_MS 1000


static void periodic_timer_callback(void* arg);
TaskHandle_t tcp_client_handle = NULL;
TaskHandle_t tcp_logging_handle = NULL;



void tcp_logging_task(void *pvParameters)
{
    xTaskCreatePinnedToCore(tcp_client_task,"tcp_client",4096,NULL,5,&tcp_client_handle,0);
    ESP_LOGI("tcp logging","task started");


    while(1) {
        ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
        // code_timer_take_timestamp(&timer, "udp_tx_task run");

        heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);

        

        // int payload = ret;
        
        // if(tcp_send(payload, 26)) {
        //     xTaskNotifyGive(tcp_client_handle);
        // }
    }
}


void app_main(void)
{
    /******* WiFi ********/
    // Initialize nvs flash to get wifi config
    ESP_ERROR_CHECK(nvs_flash_init());
    // Start wifi in station mode
    wifi_init_sta();

    
    // Wait for connection...
    vTaskDelay(5000/portTICK_PERIOD_MS);


    // Start UDP tasks
    xTaskCreatePinnedToCore(tcp_logging_task,"tcp_log_task",4096,NULL,5,&tcp_logging_handle,0);
    
    
    // Create (hardware) timer
    const esp_timer_create_args_t periodic_timer_args = {
            .callback = &periodic_timer_callback,
            /* name is optional, but may help identify the timer when debugging */
            .name = "periodic"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    /* Start the timers */
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LOGGING_INTERVAL_MS*1000));
    ESP_LOGI("timer", "Started timer");
}


static void periodic_timer_callback(void* arg)
{
    // Notify audio task
    xTaskNotifyGive(tcp_logging_handle);
}

