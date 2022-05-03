#include "i2s_driver.h"
#include "esp_check.h"
#include "esp_err.h"


QueueHandle_t i2s_evt_que;


// Monitors I2S event que and logs errors
void i2s_monitor_task(void *args)
{
    ESP_LOGI("I2S", "i2s dma monitor task started");
    i2s_event_t i2s_event;
    while(1)
    {
        xQueueReceive(i2s_evt_que, (void *)&i2s_event, portMAX_DELAY);
        switch (i2s_event.type) {
            case I2S_EVENT_DMA_ERROR:
                ESP_LOGE("I2S", "I2S_EVENT_DMA_ERROR");
                break;
            case I2S_EVENT_TX_Q_OVF:
                ESP_LOGE("I2S", "I2S_EVENT_TX_Q_OVF");
                break;
            case I2S_EVENT_RX_Q_OVF:
                ESP_LOGE("I2S", "I2S_EVENT_RX_Q_OVF");
                break;
            case I2S_EVENT_RX_DONE:
                ESP_LOGD("I2S", "I2S_EVENT_RX_DONE");
                break;
            case I2S_EVENT_TX_DONE:
                ESP_LOGD("I2S", "I2S_EVENT_TX_DONE");
                break;
            default:
                ESP_LOGE("I2S", "UNKNOWN EVENT");
                break;
        }
    }
}


void init_i2s(void) {
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX,
        .sample_rate = 48000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        // .bits_per_chan = I2S_BITS_PER_CHAN_16BIT, 
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,            /*!< The total number of descriptors used by I2S DMA to receive/transmit data */
        .dma_buf_len = 480,            /*!< Number of frames for one-time sampling. The frame here means the total data from all the channels in a WS cycle */
        .use_apll = false,
        .tx_desc_auto_clear = true,
        // .big_edin = true,
        // .mclk_multiple = I2S_MCLK_MULTIPLE_256,
        // .fixed_mclk = 1   
    };
    i2s_pin_config_t pin_config = {
        .mck_io_num = I2S_MCK_IO,           // Master clock
        .bck_io_num = I2S_BCK_IO,           // bit clock
        .ws_io_num = I2S_WS_IO,             // LR channel select
        .data_out_num = I2S_DO_IO,
        .data_in_num = I2S_DI_IO
    };
    ESP_ERROR_CHECK( i2s_driver_install(I2S_NUM, &i2s_config, 10, &i2s_evt_que) );
    ESP_ERROR_CHECK( i2s_set_pin(I2S_NUM, &pin_config) );

    // Task to monitor dma buffer overflows
    xTaskCreatePinnedToCore(i2s_monitor_task,"I2S monitor",2048,NULL,1,NULL,1);
    
    ESP_LOGI("I2S","i2s configured");
}
