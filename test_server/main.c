#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_random.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"

#define WIFI_SSID "ssid"
#define WIFI_PASS "passwrd"

static const char *TAG = "wifi_http";

//html page
static const char* html_page =
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<meta charset=\"utf-8\">"
    "<title>Losowa liczba</title>"
    "<script>"
    "function refreshData() {"
    "   fetch('/data')"
    "   .then(r => r.text())"
    "   .then(t => { document.getElementById('val').innerText = t; });"
    "}"
    "setInterval(refreshData, 5000);"
    "</script>"
    "</head>"
    "<body>"
    "<h1>Losowa liczba z zakresu 1–20</h1>"
    "<h2 id='val'>Ładowanie...</h2>"
    "</body>"
    "</html>";

//root handler
esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html_page, strlen(html_page));
    return ESP_OK;
}

//data handler
esp_err_t data_get_handler(httpd_req_t *req)
{
    char buf[16];

    // losowa liczba 1–20
    int val = (esp_random() % 20) + 1;
    sprintf(buf, "%d", val);

    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, buf, strlen(buf));
    return ESP_OK;
}

//http server
httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_uri_t root = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = root_get_handler
        };
        httpd_register_uri_handler(server, &root);

        httpd_uri_t data = {
            .uri = "/data",
            .method = HTTP_GET,
            .handler = data_get_handler
        };
        httpd_register_uri_handler(server, &data);
    }
    return server;
}

//wifi handler
static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "IP: " IPSTR, IP2STR(&event->ip_info.ip));
        start_webserver();
    }
}

//start wifi
void wifi_init_sta(void)
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
}

//main
void app_main(void)
{
    nvs_flash_init();
    wifi_init_sta();
}
