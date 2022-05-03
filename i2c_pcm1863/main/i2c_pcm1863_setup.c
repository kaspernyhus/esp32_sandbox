#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "pcm1863.h"


#define I2C_MASTER_SCL_IO           GPIO_NUM_13      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           GPIO_NUM_14      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0                /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          100000           /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

#define SAMPLE_RATE     (48000)
#define I2S_NUM         (0)
#define I2S_MCLK        (GPIO_NUM_7)
#define I2S_BCK_IO      (GPIO_NUM_4)
#define I2S_WS_IO       (GPIO_NUM_6)
#define I2S_DO_IO       (GPIO_NUM_18)
#define I2S_DI_IO       (GPIO_NUM_17)


static const char *TAG = "i2c-pcm1863-setup";

pcm1863_t adc;

/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}


static esp_err_t i2s_master_init(void)
{
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX,
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .bits_per_chan = I2S_BITS_PER_CHAN_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_MSB,
        .dma_buf_count = 8,
        .dma_buf_len = 200,
        .use_apll = true,
        // .fixed_mclk = true,
        // .mclk_multiple = I2S_MCLK_MULTIPLE_256,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1
    };
    i2s_pin_config_t pin_config = {
        .mck_io_num = I2S_MCLK,
        .bck_io_num = I2S_BCK_IO,
        .ws_io_num = I2S_WS_IO,
        .data_out_num = I2S_DO_IO,
        .data_in_num = I2S_DI_IO
    };
    esp_err_t ret = i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM, &pin_config);
    return ret;
}

void app_main(void)
{
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");

    ESP_ERROR_CHECK(i2s_master_init());
    ESP_LOGI(TAG, "i2s initialized successfully");
    float clk = i2s_get_clk(I2S_NUM);
    ESP_LOGI("I2S","CLK: %f", clk);

    pcm1863_init(&adc,PCM1863_ADDR_0,I2C_MASTER_NUM,1024,I2C_MASTER_TIMEOUT_MS);
    pcm1863_set_clk_source(&adc,PCM1863_CLK_SOURCE_XTAL);
    pcm1863_set_bit_depth(&adc,PCM1863_16BIT);
    pcm1863_get_samplerate(&adc);
    pcm1863_get_bitdepth(&adc);
    pcm1863_get_device_state(&adc);
    pcm1863_get_clk_state(&adc);
    pcm1863_get_clk_ratio(&adc);


    while(1)
    {
        // pcm1863_get_device_state(&adc);
        // pcm1863_register_read(&adc,PCM1863_FORMAT_REG,&data,1);
        // ESP_LOGI("","pcm1863 read addr: %.2X: %.2X",PCM1863_FORMAT_REG,data);

        // pcm1863_register_read(&adc,PCM1863_INFO_REG,&data,1);
        // ESP_LOGI("pcm1863","STATUS REG:%d", data);

        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}
