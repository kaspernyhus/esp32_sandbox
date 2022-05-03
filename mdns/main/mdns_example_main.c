/* MDNS-SD Query and advertise Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif_ip_addr.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "mdns.h"
#include "driver/gpio.h"
#include "netdb.h"


static const char * TAG = "mdns-test";


static void initialise_mdns(void)
{
    char * hostname = "ClassicsAdaptor";
    char * instance_name = "ClassicsAdaptor";

    //initialize mDNS
    ESP_ERROR_CHECK( mdns_init() );
    //set mDNS hostname (required if you want to advertise services)
    ESP_ERROR_CHECK( mdns_hostname_set(hostname) );
    ESP_LOGI(TAG, "mdns hostname set to: [%s]", hostname);
    //set default mDNS instance name
    ESP_ERROR_CHECK( mdns_instance_name_set(instance_name) );

    //structure with TXT records
    mdns_txt_item_t serviceTxtData[3] = {
        {"version", "1"},
        {"product", "beosound4"},
        {"format", "48kHz/16"}
    };

    //initialize service
    ESP_ERROR_CHECK( mdns_service_add(instance_name, "_classicsadaptorstream", "_udp", 5004, serviceTxtData, 3) );
}


void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    initialise_mdns();


    ESP_ERROR_CHECK(example_connect());


}

