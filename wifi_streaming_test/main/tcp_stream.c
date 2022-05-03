#include "tcp_stream.h"
#include "esp_log.h"



static const char *TCP_TAG = "tcp";
// static const char *payload = "Message from ESP32 ";

static int sock = 0;


esp_err_t tcp_send(const void *payload, size_t len) {
    int err = send(sock, payload, len, 0);
    if (err < 0) {
        ESP_LOGE(TCP_TAG, "Error occurred during sending: errno %d", errno);
        return ESP_FAIL;
    }
    return ESP_OK;
}


void tcp_client_task(void *pvParameters)
{
    char rx_buffer[128];
    char host_ip[] = TCP_HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;

    while (1) {
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(host_ip);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(TCP_PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;

        sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TCP_TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TCP_TAG, "Socket created (%d), trying to connect to %s:%d", sock, host_ip, TCP_PORT);

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in6));
        if (err != 0) {
            ESP_LOGE(TCP_TAG, "Socket unable to connect: errno %d", errno);
            break;
        }
        ESP_LOGI(TCP_TAG, "Successfully connected");

        while (1) {
            // int err = send(sock, payload, strlen(payload), 0);
            // if (err < 0) {
            //     ESP_LOGE(TCP_TAG, "Error occurred during sending: errno %d", errno);
            //     break;
            // }

            // int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            // // Error occurred during receiving
            // if (len < 0) {
            //     ESP_LOGE(TCP_TAG, "recv failed: errno %d", errno);
            //     break;
            // }
            // // Data received
            // else {
            //     rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
            //     ESP_LOGI(TCP_TAG, "Received %d bytes from %s:", len, host_ip);
            //     ESP_LOGI(TCP_TAG, "%s", rx_buffer);
            // }

            if(ulTaskNotifyTake(pdTRUE,portMAX_DELAY)) {
                ESP_LOGE(TCP_TAG,"Error occurred in tcp client task");
                break;
            }

            // vTaskDelay(1000 / portTICK_PERIOD_MS);
        }

        if (sock != -1) {
            ESP_LOGE(TCP_TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}