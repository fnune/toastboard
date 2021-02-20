/* Simple HTTP Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>

#include <esp_http_server.h>

#include "esp8266/rtc_register.h"
#include "memfault/components.h"
#include "memfault/esp8266_port/http_client.h"
#include "memfault/core/reboot_tracking.h"

// An opaque storage area used by the Memfault SDK to track reboot information.
static uint8_t s_reboot_tracking[MEMFAULT_REBOOT_TRACKING_REGION_SIZE];

/* A simple example that demonstrates how to create GET and POST
 * handlers for the web server.
 * The examples use simple WiFi configuration that you can set via
 * 'make menuconfig'.
 * If you'd rather not, just change the below entries to strings
 * with the config you want -
 * ie. #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_WIFI_SSID CONFIG_WIFI_SSID
#define EXAMPLE_WIFI_PASS CONFIG_WIFI_PASSWORD

// From components/esp8266/source/reset_reason.c
#define RTC_RESET_HW_CAUSE_REG RTC_STATE1

static const char *TAG="TOASTBOARD";

/* An HTTP GET handler */
esp_err_t upload_get_handler(httpd_req_t *req)
{
    memfault_esp_port_http_client_post_data();

    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, strlen(resp_str));

    return ESP_OK;
}

httpd_uri_t upload = {
    .uri       = "/upload",
    .method    = HTTP_GET,
    .handler   = upload_get_handler,
    .user_ctx  = "Uploading stuff to Memfault!"
};

esp_err_t crash_get_handler(httpd_req_t *req)
{
    while (true) {
      // ...
    };

    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, strlen(resp_str));

    return ESP_OK;
}

httpd_uri_t crash = {
    .uri       = "/crash",
    .method    = HTTP_GET,
    .handler   = crash_get_handler,
    .user_ctx  = "The application should crash before this shows up"
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
        httpd_register_uri_handler(server, &upload);
        httpd_register_uri_handler(server, &crash);
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

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    httpd_handle_t *server = (httpd_handle_t *) ctx;
    /* For accessing reason codes in case of disconnection */
    system_event_info_t *info = &event->event_info;

    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
        ESP_ERROR_CHECK(esp_wifi_connect());
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
        ESP_LOGI(TAG, "Got IP: '%s'",
                ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));

        /* Start the web server */
        if (*server == NULL) {
            *server = start_webserver();
        }
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
        ESP_LOGE(TAG, "Disconnect reason : %d", info->disconnected.reason);
        if (info->disconnected.reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT) {
            /*Switch to 802.11 bgn mode */
            esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
        }
        ESP_ERROR_CHECK(esp_wifi_connect());

        /* Stop the web server */
        if (*server) {
            stop_webserver(*server);
            *server = NULL;
        }
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void initialise_wifi(void *arg)
{
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, arg));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_WIFI_SSID,
            .password = EXAMPLE_WIFI_PASS,
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

eMemfaultRebootReason get_memfault_reset_reason(void)
{
    const esp_reset_reason_t reason = esp_reset_reason();
    switch (reason) {
        /* Reset after exiting deep sleep mode. */
        case ESP_RST_DEEPSLEEP: return kMfltRebootReason_DeepSleep;
        /* Reset due to power-on event. */
        case ESP_RST_POWERON: return kMfltRebootReason_PowerOnReset;
        /* Software reset via esp_restart. */
        case ESP_RST_SW: return kMfltRebootReason_SoftwareReset;
        /* Reset over SDIO. */
        case ESP_RST_SDIO: return kMfltRebootReason_UserReset;
        /* Brownout reset (software or hardware) */
        case ESP_RST_BROWNOUT: return kMfltRebootReason_BrownOutReset;

        /* Software reset due to exception/panic. */
        case ESP_RST_PANIC: return kMfltRebootReason_UnknownError;

        /* Reset (software or hardware) due to interrupt watchdog. */
        case ESP_RST_INT_WDT: return kMfltRebootReason_HardwareWatchdog;
        /* Reset due to task watchdog. */
        case ESP_RST_TASK_WDT: return kMfltRebootReason_SoftwareWatchdog;
        /* Reset due to other watchdogs. */
        case ESP_RST_WDT: return kMfltRebootReason_HardwareWatchdog;

        /* Reset reason can not be determined. */
        case ESP_RST_UNKNOWN: return kMfltRebootReason_Unknown;

        default: return kMfltRebootReason_Unknown;
    }
}

static void initialise_reboot_tracking(void) {
    const sResetBootupInfo reset_reason = {
       .reset_reason_reg = RTC_RESET_HW_CAUSE_REG,
       .reset_reason = get_memfault_reset_reason(),
    };

    memfault_reboot_tracking_boot(s_reboot_tracking, &reset_reason);

    static uint8_t s_event_storage[100];
    const sMemfaultEventStorageImpl *evt_storage = memfault_events_storage_boot(s_event_storage, sizeof(s_event_storage));
    memfault_reboot_tracking_collect_reset_info(evt_storage);
}

void app_main()
{
    memfault_platform_boot();
    initialise_reboot_tracking();

    static httpd_handle_t server = NULL;
    ESP_ERROR_CHECK(nvs_flash_init());
    initialise_wifi(&server);
}
