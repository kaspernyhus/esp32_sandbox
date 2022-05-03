#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "rmt_uart.h"
#include "driver/rmt.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#define RMT_UART_TX_GPIO    GPIO_NUM_18
#define RMT_UART_RX_GPIO    GPIO_NUM_17
#define RMT_TX_CHANNEL RMT_CHANNEL_0


#define RMT_BUF_SZ 20*32
uint8_t rmt_buf[RMT_BUF_SZ];



void print_item_contens(rmt_item32_t *items, size_t item_num)
{
    ESP_LOGI("","------------------------------");
    for(int i=0; i<item_num; i++) {
        ESP_LOGI("item","0x%.4X",items[i].val);
        
        ESP_LOGI("item","Duration0: %d",items[i].duration0);
        ESP_LOGI("item","Duration1: %d",items[i].duration1);
        ESP_LOGI("item","Level0: %d",items[i].level0);
        ESP_LOGI("item","Level1: %d",items[i].level1);
    }
}



void app_main(void)
{
    rmt_uart_config_t rmt_uart_cfg = {
        .baud_rate = 19200,
        .mode = RMT_UART_MODE_TX_ONLY,
        .data_bits = RMT_UART_DATA_9_BITS,
        .parity = RMT_UART_PARITY_DISABLE,
        .stop_bits = RMT_UART_STOP_BITS_1,
        .rx_io_num = RMT_UART_RX_GPIO,
        .tx_io_num = RMT_UART_TX_GPIO,
        .rx_buffer_size = 2048,
        .tx_items_buffer_size = 100,
        .rmt_tx_channel = RMT_CHANNEL_0,
        .rmt_rx_channel = RMT_CHANNEL_4
    };

    rmt_uart_init(RMT_UART_NUM_0, &rmt_uart_cfg);

    uint8_t test_bytes[] = {0x01, 0x83, 0x00, 0x38, 0x01, 0xAB};

    while(1)
    {

        rmt_uart_write_bytes(RMT_UART_NUM_0, test_bytes, sizeof(test_bytes));

        // ESP_LOGI("rmt", "Transmission complete");
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}
