/*
 *  B&O Classics Adaptor
 *
 *  UDP streming module
 *
 *  LAPJ/KANK
 *  2022
 * 
*/

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#define UDP_HOST_IP_ADDR "192.168.1.137"
#define UDP_PORT 5004


typedef struct {
    uint8_t *tx_buffer;
    size_t length;
} udp_data_t;


esp_err_t udp_write(const void *payload, size_t len);
// void udp_write(uint8_t *data, size_t length);
void udp_client_task(void *pvParameters);