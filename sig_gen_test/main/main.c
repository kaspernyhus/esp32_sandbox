#include <stdio.h>

#include "esp_log.h"
#include "sig_gen.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


sig_gen_t sigGenL;
sig_gen_t sigGenR;

#define SAMPLES_NUM 4096
#define BYTES_PER_SAMPLE 2


#define BUF_SZ SAMPLES_NUM*BYTES_PER_SAMPLE*2
uint8_t buffer[BUF_SZ];


void app_main(void)
{   
    sig_gen_config_t sigGen_config = {
        // .gen_source = LUT_GEN,
        // .lut_freq = LUT_FREQ_1K,
        .gen_source = WHITE_NOISE,
        // .freq = 2000,
        // .amplitude = 1,
        // .phase = 0,
        .sample_rate = 48000,
        .bytes_per_sample = BYTES_PER_SAMPLE,
        .endianess = SIG_GEN_LE
    };
    sig_gen_init(&sigGenL,&sigGen_config);
    // sigGen_config.lut_freq = LUT_FREQ_5K3;
    // sigGen_config.freq = 329;
    sig_gen_init(&sigGenR,&sigGen_config);

    sig_gen_output_combine(&sigGenL, &sigGenR, buffer, SAMPLES_NUM);
    ESP_LOG_BUFFER_HEX("buffer", buffer, BUF_SZ);

    while(1)
    {

        vTaskDelay(100);
    }
}
