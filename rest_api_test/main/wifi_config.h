/*
 *  B&O Classics Adaptor
 *
 *  WiFI configuration module
 *
 *  LAPJ/KANK
 *  2022
 * 
*/

#pragma once

#include "esp_wifi.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"

#include "lwip/err.h"
#include "lwip/sys.h"

// #define EXAMPLE_ESP_WIFI_SSID      "esp32_network"
// #define EXAMPLE_ESP_WIFI_PASS      "esp32ernice"
// #define EXAMPLE_ESP_WIFI_SSID      "Lyngby-4G-4"
// #define EXAMPLE_ESP_WIFI_PASS      "Play1122"
// #define EXAMPLE_ESP_WIFI_SSID      "nyhus_dLink_2.4"
// #define EXAMPLE_ESP_WIFI_PASS      "kaspernyhus1234"
#define EXAMPLE_ESP_WIFI_SSID      "LRKSPR_5G"
#define EXAMPLE_ESP_WIFI_PASS      "honeybear"
// #define EXAMPLE_ESP_WIFI_SSID      "3DK-B535-9775"
// #define EXAMPLE_ESP_WIFI_PASS      "4BAY7503N5J"
// #define EXAMPLE_ESP_WIFI_SSID      "Just-JensenNr3"
// #define EXAMPLE_ESP_WIFI_PASS      "Sydkaeret3"

#define EXAMPLE_ESP_MAXIMUM_RETRY  5




void wifi_init_sta(void);