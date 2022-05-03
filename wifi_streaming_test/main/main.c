/*
 *  B&O Classics Adaptor
 *
 *  WiFI streaming test
 *
 *  LAPJ/KANK
 *  2022
 * 
*/

#include <stdio.h>
#include "wifi_config.h"
#include "nvs_flash.h"
#include "udp_stream.h"
// #include "tcp_stream.h"
#include "rtptx.h"
#include "esp_timer.h"

#include "CA_i2s.h"
#include "CA_i2c.h"
#include "pcm1863.h"
#include "circular_buffer.h"

#include "sig_gen.h"
#include "code_timer.h"
#include "log_buffer.h"


#define SAMPLE_RATE 48000
#define BYTES_PER_SAMPLE 2
#define CHANNELS 2
#define TIME_SLICE_IN_MS 10

#define SAMPLES_PER_TIME_SLICE ( (SAMPLE_RATE/1000) * TIME_SLICE_IN_MS * CHANNELS)
#define BYTES_PER_TIME_SLICE ( SAMPLES_PER_TIME_SLICE * BYTES_PER_SAMPLE )
#define AUDIO_BUF_SZ ( BYTES_PER_TIME_SLICE )


// DEBUG
#define LOG_BUF_SZ AUDIO_BUF_SZ*4
uint8_t logbuffer[LOG_BUF_SZ];
uint8_t logbuffer2[LOG_BUF_SZ];
log_buffer_t logbuf;
log_buffer_t logbuf2;
code_timer_t timer;



pcm1863_t adc;

#define R_BUF_SZ ( (BYTES_PER_TIME_SLICE+RTP_HEADER_LEN)*50 )
uint8_t rbuffer[R_BUF_SZ];
ringbuf_t rbuf;

uint8_t pcm_data[AUDIO_BUF_SZ];
uint8_t pcm_data_BE[AUDIO_BUF_SZ];
uint8_t rtp_data[AUDIO_BUF_SZ+RTP_HEADER_LEN];
uint8_t udp_data[AUDIO_BUF_SZ+RTP_HEADER_LEN];

static void periodic_timer_callback(void* arg);
TaskHandle_t audio_task_handle = NULL;
TaskHandle_t udp_task_handle = NULL;
TaskHandle_t udp_tx_handle = NULL;


void audio_task(void *pvParameters)
{   
    ESP_LOGI("audio","starting audio task: sample rate=%d, bytes_per_sample=%d, SAMPLES_PER_TIME_SLICE=%d, BYTES_PER_TIME_SLICE=%d, AUDIO_BUF_SZ=%d",SAMPLE_RATE,BYTES_PER_SAMPLE,SAMPLES_PER_TIME_SLICE,BYTES_PER_TIME_SLICE,AUDIO_BUF_SZ);
    size_t i2s_bytes_read;
    size_t bytes_written;
    while (1)
    {
        esp_err_t ret = i2s_read(I2S_NUM,pcm_data,BYTES_PER_TIME_SLICE,&i2s_bytes_read,portMAX_DELAY);
        if (ret != ESP_OK) {
            ESP_LOGE("I2S", "i2s read failed");
        }
        // code_timer_take_timestamp(&timer, "i2s_read done     ");

        rtp_to16BE(pcm_data,pcm_data_BE,SAMPLES_PER_TIME_SLICE);
        // code_timer_take_timestamp(&timer, "rtp_to16BE done   ");
        
        size_t rtp_len = rtp_create_packet(RTP_SESSION_NUM_0,pcm_data_BE,BYTES_PER_TIME_SLICE,rtp_data);
        // code_timer_take_timestamp(&timer, "rtp packet created");


        bytes_written = ringbuf_write(&rbuf, rtp_data, rtp_len);
        if (bytes_written != rtp_len) {
            ESP_LOGE("audio_task", "failed to write all bytes to ringbuffer");
        }
        
        // xTaskNotifyGive(udp_tx_handle);
        
        // code_timer_take_timestamp(&timer, "audio_task done   ");
    }
}


void udp_tx_task(void *pvParameters)
{
    ESP_LOGI("audio","udp tx task started");
    size_t bytes_read;
    while(1) {
        ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
        // code_timer_take_timestamp(&timer, "udp_tx_task run");

        bytes_read = ringbuf_read(&rbuf,udp_data,BYTES_PER_TIME_SLICE+RTP_HEADER_LEN);
        if(bytes_read != BYTES_PER_TIME_SLICE+RTP_HEADER_LEN) {
            ESP_LOGE("rbuf","Error. Not enough bytes read from ringbuffer");
        }
        
        if(udp_write(udp_data,bytes_read) != ESP_OK) {
            // ESP_LOGW("audio","Notifying udp task of error");
        }
    }
}


void app_main(void)
{
    /******* Debug ********/
    code_timer_init(&timer, "audio task notified",/*buffer size*/100,/*when to display*/100);
    log_buffer_init(&logbuf,logbuffer,LOG_BUF_SZ,100,"PCM LE");
    log_buffer_init(&logbuf2,logbuffer2,LOG_BUF_SZ,1000,"PCM BE");


    /******* WiFi ********/
    // Initialize nvs flash to get wifi config
    ESP_ERROR_CHECK(nvs_flash_init());
    // Start wifi in station mode
    wifi_init_sta();

    // FIX: abort() if failed to connect to WiFi

    // Initialize RTP session
    rtp_session_init(RTP_SESSION_NUM_0,L16);
    
    // Wait for connection...
    vTaskDelay(5000/portTICK_PERIOD_MS);



    /******* Audio ********/
    ringbuf_init(&rbuf,rbuffer,R_BUF_SZ);
    init_i2s();

    // ADC
    i2c_master_init();
    pcm1863_init(&adc,PCM1863_ADDR_0,I2C_MASTER_NUM,1024,I2C_MASTER_TIMEOUT_MS);
    pcm1863_set_clk_source(&adc,PCM1863_CLK_SOURCE_XTAL);
    pcm1863_set_bit_depth(&adc,PCM1863_16BIT);
    pcm1863_get_samplerate(&adc);
    pcm1863_get_bitdepth(&adc);
    pcm1863_get_device_state(&adc);


    // Start UDP tasks
    xTaskCreatePinnedToCore(udp_client_task,"udp_client",4096,NULL,5,&udp_task_handle,0);
    xTaskCreatePinnedToCore(udp_tx_task,"udp_tx",4096,NULL,5,&udp_tx_handle,0);
    
    // Start audio task
    xTaskCreatePinnedToCore(audio_task,"audio task",4096*2,NULL,6,&audio_task_handle,1);
    
    // Create (hardware) timer
    const esp_timer_create_args_t periodic_timer_args = {
            .callback = &periodic_timer_callback,
            /* name is optional, but may help identify the timer when debugging */
            .name = "periodic"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    /* Start the timers */
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, TIME_SLICE_IN_MS*1000));
    ESP_LOGI("timer", "Started timer");
}


static void periodic_timer_callback(void* arg)
{
    // Notify audio task
    xTaskNotifyGive(udp_tx_handle);
}


// GST_DEBUG=3 gst-launch-1.0 udpsrc port=5004 ! 'application/x-rtp,media=audio,payload=96,clock-rate=48000,encoding-name=L16,channels=2' ! rtpjitterbuffer ! rtpL16depay ! queue min-threshold-time=10000000 ! audioresample ! autoaudiosink sync=0