#include <stdio.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"

#include "sig_gen.h"
#include "remote_log.h"
#include "esp_heap_caps.h"


#define SAMPLES_NUM 128
#define BYTES_PER_SAMPLE 2

#define BUF_SZ SAMPLES_NUM*BYTES_PER_SAMPLE*2
#define RING_BUF_BYTES BUF_SZ*8

uint8_t write_buffer[BUF_SZ];
uint8_t read_buffer[BUF_SZ];
size_t bytes_read;

RingbufHandle_t rbuf_handle = NULL;


esp_err_t log_heap(void *out, size_t *len)
{
    size_t data = heap_caps_get_free_size(MALLOC_CAP_8BIT);

    // boilerplate   
    memcpy(out,&data,sizeof(data));
    *len = sizeof(data);
    return ESP_OK;
}

esp_err_t log_xring(void *out, size_t *len)
{
    size_t data = xRingbufferGetCurFreeSize(rbuf_handle);

    // boilerplate   
    memcpy(out,&data,sizeof(data));
    *len = sizeof(data);
    return ESP_OK;
}


void write_task(void *args)
{
   
    while(1)
    {
        sig_gen_ez_read(write_buffer, SAMPLES_NUM, &bytes_read);
        ESP_LOG_BUFFER_HEX("buffer", write_buffer, BUF_SZ);
        
        UBaseType_t res =  xRingbufferSend(rbuf_handle, write_buffer, sizeof(write_buffer), portMAX_DELAY);
        if (res != pdTRUE) {
            ESP_LOGE("write_task","Failed to send item");
        }
    }
}

void read_task(void *args)
{
    size_t item_size;
    
    while(1)
    {
        char *item = (char *)xRingbufferReceive(rbuf_handle, &item_size, portMAX_DELAY);
        
        if (item != NULL) {
            // ESP_LOG_BUFFER_HEX("buffer", item, item_size);
            vRingbufferReturnItem(rbuf_handle, (void *)item);
        } else {
            ESP_LOGE("read_task","Failed to receive item");
        }
    }
}


void app_main(void)
{   
    remote_log_config cfg = {
        .transport_type = REMOTE_LOG_UART,
        .uart_num = 1,
        .baud_rate = 9600,
        .tx_pin = 17,
        .rx_pin = 18,
        .log_frequency_ms = 100
    };
    remote_log_init(&cfg);

    remote_log_register_t heap_log = {
        .log_id = 0x01,
        .tag = "Heap tracking",
        .data_log_cb = log_heap
    };
    remote_log_register_log(heap_log);

    remote_log_register_t xring_log = {
        .log_id = 0x02,
        .tag = "xRing free space",
        .data_log_cb = log_xring
    };
    remote_log_register_log(xring_log);


    sig_gen_ez_1k_stereo_init(48000, BYTES_PER_SAMPLE, SIG_GEN_ENABLE_CB, 1000);

    StaticRingbuffer_t *buffer_struct = (StaticRingbuffer_t *)heap_caps_malloc(sizeof(StaticRingbuffer_t), MALLOC_CAP_SPIRAM);
    uint8_t *buffer_storage = (uint8_t *)heap_caps_malloc(sizeof(uint8_t)*RING_BUF_BYTES, MALLOC_CAP_SPIRAM);
    RingbufferType_t buffer_type = RINGBUF_TYPE_NOSPLIT;
    rbuf_handle = xRingbufferCreateStatic(RING_BUF_BYTES, buffer_type, buffer_storage, buffer_struct);
    
    xTaskCreatePinnedToCore(write_task,"WriteTask",4000,NULL,5,NULL,1);
    xTaskCreatePinnedToCore(read_task,"ReadTask",4000,NULL,5,NULL,1);
}
