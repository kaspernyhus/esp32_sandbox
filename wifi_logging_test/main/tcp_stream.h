#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "lwip/err.h"
#include "lwip/sockets.h"

#define TCP_HOST_IP_ADDR "192.168.0.85"
#define TCP_PORT 3333



void tcp_client_task(void *pvParameters);
esp_err_t tcp_send(const void *payload, size_t len);
