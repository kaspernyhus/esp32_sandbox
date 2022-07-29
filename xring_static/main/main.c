#include <stdio.h>

#include "esp_log.h"
#include "sig_gen.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#define SAMPLES_NUM 128
#define BYTES_PER_SAMPLE 2

#define BUF_SZ SAMPLES_NUM*BYTES_PER_SAMPLE*2
uint8_t buffer[BUF_SZ];
size_t bytes_read;


void write_task(void *args)
{
    while(1)
    {
        sig_gen_ez_read(buffer, SAMPLES_NUM, &bytes_read);
        ESP_LOG_BUFFER_HEX("buffer", buffer, BUF_SZ);
    }
}

void read_task(void *args)
{
    while(1)
    {
        vTaskDelay(100);
    }
}


void app_main(void)
{   
    sig_gen_ez_1k_stereo_init(48000, BYTES_PER_SAMPLE, SIG_GEN_ENABLE_CB, 1000);
    
    xTaskCreatePinnedToCore(write_task,"WriteTask",4000,NULL,5,NULL,1);
}
