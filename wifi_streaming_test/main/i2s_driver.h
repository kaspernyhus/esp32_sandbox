/*
 *  B&O Classics Adaptor
 *
 *  i2s module
 *
 *  LAPJ/KANK
 *  2022
 * 
*/


#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"
 
#include "driver/i2s.h"
#include "driver/gpio.h"



#define I2S_NUM         (I2S_NUM_0)
#define I2S_MCK_IO      (GPIO_NUM_7)
#define I2S_BCK_IO      (GPIO_NUM_4)
#define I2S_WS_IO       (GPIO_NUM_6)
#define I2S_DO_IO       (GPIO_NUM_18)
#define I2S_DI_IO       (GPIO_NUM_17)



void init_i2s(void);
void create_i2s_task();