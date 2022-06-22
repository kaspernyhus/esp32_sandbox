#pragma once

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <esp_http_server.h>


esp_err_t start_webserver(void);
void stop_webserver(httpd_handle_t server);

esp_err_t webserver_register_uri(char* uri, httpd_method_t method, esp_err_t (*handler)(httpd_req_t *r));