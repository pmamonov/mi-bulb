/* Simple HTTP Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>

#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "protocol_examples_common.h"
#include "nvs.h"
#include "nvs_flash.h"

#include <esp_http_server.h>

#include "main.h"

static const char *TAG="HTTP";

static const char *resp =
"<html><body>"
"<h1>%d%%</h1>"
"<form action='/' method='get'>"
"<input type='number' name='br'></input>"
"<input type='submit' value='OK'>"
"</form>"
"<hr><h1>Update firmware</h1>"
"<form action='/update' method='get'>"
"Host: <input name='h'></input><br>"
"Port: <input name='p'></input><br>"
"FIXME (url_decode): File path: <input name='f'></input><br>"
"<input type='submit' value='UPDATE'>"
"</form>"
"</body></html>";

/* An HTTP GET handler */
esp_err_t root_get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;

    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            char param[32];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "br", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "br=`%s`", param);
                br = strtoul(param, NULL, 10) % 101;
            }
        }
        free(buf);
    }

    buf_len = strlen(resp) + 10;
    buf = malloc(buf_len);
    snprintf(buf, buf_len, resp, br);
    httpd_resp_send(req, buf, strlen(buf));
    free(buf);

    return ESP_OK;
}

httpd_uri_t root = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = root_get_handler,
};


static void ota_var_init(char *buf, char *key, char **ota_var)
{
    char param[128];

    if (httpd_query_key_value(buf, key, param, sizeof(param)) == ESP_OK) {
        ESP_LOGI(TAG, "%s: %s=`%s`\n", __func__,  key, param);
        *ota_var = malloc(1 + strlen(param));
        if (!*ota_var) {
            ESP_LOGI(TAG, "%s: malloc failed\n", __func__);
            return;
        }
        memcpy(*ota_var, param, 1 + strlen(param));
    }
}

static esp_err_t update_get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;

    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ota_var_init(buf, "h", &ota_host);
            ota_var_init(buf, "p", &ota_port);
            ota_var_init(buf, "f", &ota_file);
        }
        free(buf);
    }

    if (ota_host && ota_port && ota_file) {
        httpd_resp_send(req, "OK", 2);
        start_ota();
    } else {
        if (ota_host)
            free(ota_host);
        if (ota_port)
            free(ota_port);
        if (ota_file)
            free(ota_file);
        ota_host = ota_port = ota_file = NULL;
        httpd_resp_send(req, "FAIL", 4);
    }

    return ESP_OK;
}

static httpd_uri_t update = {
    .uri       = "/update",
    .method    = HTTP_GET,
    .handler   = update_get_handler,
};

httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &update);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

static httpd_handle_t server = NULL;

static void disconnect_handler(void* arg, esp_event_base_t event_base, 
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base, 
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

void start_http()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());


    ESP_ERROR_CHECK(example_connect());

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));

    server = start_webserver();
}
