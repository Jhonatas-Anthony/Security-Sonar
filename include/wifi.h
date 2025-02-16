#ifndef WIFI_H
#define WIFI_H

#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"

#define SERVER_URL "embarcatech.icy-tree-310a.workers.dev"

void wifi_connect(char *ssid, char *pass);
void send_custom_http_request(const char *method, const char *endpoint, const char *body);
bool is_response_complete();
char *get_response_buffer();
char *set_response_buffer(char *buffer);
void set_response_complete(bool status);
void wifi_cleanup();
void send_request_to_change_status(int status);

#endif