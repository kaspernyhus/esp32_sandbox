#include <stdio.h>
#include "esp_log.h"
#include "esp_lc3plus.h"
#include "sig_gen.h"

#define SAMPLES_NUM 480
#define BYTES_PER_SAMPLE 3
#define BYTES_PER_FRAME ( SAMPLES_NUM * BYTES_PER_SAMPLE )

sig_gen_t sigGenL;
sig_gen_t sigGenR;
int32_t samples[BYTES_PER_FRAME];
uint8_t lc3binary[20000];


void app_main(void)
{
    sig_gen_config_t cfg = {
        .gen_source = SINE_LUT,
        .lut_freq = LUT_FREQ_1K,
        .bytes_per_sample = 3,
        .sample_rate = 48000,
        .endianess = SIG_GEN_LE
    };
    sig_gen_init(&sigGenL, &cfg);
    sig_gen_init(&sigGenR, &cfg);

    sig_gen_output_combine(&sigGenL, &sigGenR, samples, SAMPLES_NUM);

    ESP_LOG_BUFFER_HEX("pre", samples, BYTES_PER_FRAME);

    lc3plus_init();
    
    lc3plus_enc(samples, lc3binary);

    ESP_LOG_BUFFER_HEX("post", lc3binary, BYTES_PER_FRAME);
}
