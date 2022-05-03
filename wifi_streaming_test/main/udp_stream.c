#include <string.h>
#include "udp_stream.h"
#include "esp_log.h"


static const char *UDP_TAG = "udp";

static int sock = 0;
static struct sockaddr_in dest_addr;


esp_err_t udp_write(const void *payload, size_t len) {
    int err = sendto(sock, payload, len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
        ESP_LOGE(UDP_TAG, "Error occurred during sending: errno %d", errno);
        return ESP_FAIL;
    }
    return ESP_OK;
}

void udp_client_task(void *pvParameters)
{
    char host_ip[] = UDP_HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;

    while (1) {
        // struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(UDP_HOST_IP_ADDR);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(UDP_PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;

        sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(UDP_TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        int ttl = 64;
        setsockopt(sock, ip_protocol, IP_TTL, &ttl, sizeof(ttl));

    
        ESP_LOGI(UDP_TAG, "Socket created (%d), sending to %s:%d", sock, UDP_HOST_IP_ADDR, UDP_PORT);


        while (1) {
            if(ulTaskNotifyTake(pdTRUE,portMAX_DELAY)) {
                ESP_LOGE(UDP_TAG,"Error occurred in udp client task");
                break;
            }
        }

        if (sock != -1) {
            ESP_LOGE(UDP_TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}