#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "esp_log.h"


#define PCM1863_ADDR_0              0x94
#define PCM1863_ADDR_1              0x96

/* PCM1863 register addresses */
#define PCM1863_INFO_REG            0x73  // Get current sampling frequency as 3 bit range (pp. 126)
#define PCM1863_POWER_STATUS_REG    0x78  // 111 = all good
#define PCM1863_FORMAT_REG          0x0B
#define PCM1863_STATE               0x72
#define PCM1863_CLK_RATIO           0x74
#define PCM1863_CLK_ERR_STAT        0x75
#define PCM1863_CLK_SOURCE          0x20
#define PCM1863_GPIO3_2_CTRL        0x11
#define PCM1863_GPIO3_2_DIR_CTRL    0x13
#define PCM1863_GPIO_IN_OUT         0x14


/* PCM1863 modes */
#define PCM1863_MODE_SLAVE          0x00
#define PCM1863_MODE_MASTER         0x01

typedef enum {
  PCM1863_CLK_SOURCE_SCK_OR_XTAL = 0,
  PCM1863_CLK_SOURCE_SCK,
  PCM1863_CLK_SOURCE_XTAL
} pcm1863_clk_source;


typedef struct {
  uint8_t i2c_channel;
  uint16_t i2c_timeout;
  uint32_t i2c_trans_buf_sz;
  uint8_t device_address;
} pcm1863_t;


typedef enum {
  PCM1863_32BIT = 0x00,
  PCM1863_24BIT,
  PCM1863_20BIT,
  PCM1863_16BIT
} pcm1863_bit_depth_t;



esp_err_t pcm1863_init(pcm1863_t *pcm1863, uint8_t device_address, uint8_t i2c_channel, uint32_t i2c_trans_buf_sz, uint16_t timeout);
esp_err_t pcm1863_register_read(pcm1863_t *pcm1863, uint8_t reg_addr, uint8_t *data, size_t len);
esp_err_t pcm1863_register_write_byte(pcm1863_t *pcm1863, uint8_t reg_addr, uint8_t data);
esp_err_t pcm1863_register_update_bits(pcm1863_t *pcm1863, uint8_t reg_addr, uint8_t mask, uint8_t val);

esp_err_t pcm1863_set_bit_depth(pcm1863_t *pcm1863, pcm1863_bit_depth_t bit_depth);
esp_err_t pcm1863_set_clk_source(pcm1863_t *pcm1863, pcm1863_clk_source clk_source);
esp_err_t pcm1863_set_mode(pcm1863_t *pcm1863, size_t mode);
void pcm1863_change_page(pcm1863_t *pcm1863, size_t page);

void pcm1863_reset(pcm1863_t *pcm1863);
void pcm1863_get_power_status(pcm1863_t *pcm1863);
void pcm1863_get_samplerate(pcm1863_t *pcm1863);
void pcm1863_get_bitdepth(pcm1863_t *pcm1863);
void pcm1863_get_device_state(pcm1863_t *pcm1863);
void pcm1863_get_clk_state(pcm1863_t *pcm1863);
void pcm1863_get_clk_ratio(pcm1863_t *pcm1863);
void pcm1863_get_mode(pcm1863_t *pcm1863);