#include "opcua_esp32.h"
#include "model.h"
#include "driver/i2c_master.h"
#include "sensor_utils.h"

#define TAG "OPCUA_ESP32"
#define SNTP_TAG "SNTP"
#define MEMORY_TAG "MEMORY"
#define ENABLE_MDNS 1


static bool obtain_time(void);
static void initialize_sntp(void);

UA_ServerConfig *config;
static UA_Boolean sntp_initialized = false;
static UA_Boolean running = true;
static UA_Boolean isServerCreated = false;
RTC_DATA_ATTR static int boot_count = 0;
static struct tm timeinfo;
static time_t now = 0;

static UA_StatusCode
UA_ServerConfig_setUriName(UA_ServerConfig *uaServerConfig, const char *uri, const char *name)
{
    UA_String_clear(&uaServerConfig->applicationDescription.applicationUri);
    UA_LocalizedText_clear(&uaServerConfig->applicationDescription.applicationName);

    uaServerConfig->applicationDescription.applicationUri = UA_String_fromChars(uri);
    uaServerConfig->applicationDescription.applicationName.locale = UA_STRING_NULL;
    uaServerConfig->applicationDescription.applicationName.text = UA_String_fromChars(name);

    for (size_t i = 0; i < uaServerConfig->endpointsSize; i++)
    {
        UA_String_clear(&uaServerConfig->endpoints[i].server.applicationUri);
        UA_LocalizedText_clear(
            &uaServerConfig->endpoints[i].server.applicationName);

        UA_String_copy(&uaServerConfig->applicationDescription.applicationUri,
                       &uaServerConfig->endpoints[i].server.applicationUri);

        UA_LocalizedText_copy(&uaServerConfig->applicationDescription.applicationName,
                              &uaServerConfig->endpoints[i].server.applicationName);
    }

    return UA_STATUSCODE_GOOD;
}

static void opcua_task(void *arg)
{
    UA_Int32 sendBufferSize = 8192;
    UA_Int32 recvBufferSize = 8192;
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));

    ESP_LOGI(TAG, "Fire up OPC UA Server.");
    UA_Server *server = UA_Server_new();
    UA_ServerConfig *config = UA_Server_getConfig(server);

    ESP_LOGI(TAG, "Ustawiono maxSessions na: %d", (int)config->maxSessions);
    UA_ServerConfig_setMinimalCustomBuffer(config, 4840, 0, sendBufferSize, recvBufferSize);

    const char *appUri = "open62541.esp32.server";
    UA_String hostName = UA_STRING("opcua-esp32");
    
    UA_ServerConfig_setUriName(config, appUri, "OPC_UA_BLACKBOX");
    UA_ServerConfig_setCustomHostname(config, hostName);
    config->maxSessions = 1;
    addSensorNodes(server);

    ESP_LOGI(TAG, "Heap Left : %d", xPortGetFreeHeapSize());
    UA_StatusCode retval = UA_Server_run_startup(server);

    i2c_master_bus_handle_t bus;
    i2c_master_dev_handle_t h_light, h_pressure;

    bus_init(&bus);
    pressure_sensor_init(&bus, &h_pressure);
    light_sensor_init(bus, &h_light);

    if (retval == UA_STATUSCODE_GOOD)
    {
        while (running)
        {
            float t = read_temp_pressure(h_pressure);
            float lux = read_light(h_light);

            setTemperatureValue(server, t);
            setLightValue(server, lux);
            UA_Server_run_iterate(server, false);

            ESP_ERROR_CHECK(esp_task_wdt_reset());
            vTaskDelay(pdMS_TO_TICKS(200)); 
        }
            UA_Server_run_shutdown(server);
    }
    ESP_ERROR_CHECK(esp_task_wdt_delete(NULL));
}

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(SNTP_TAG, "Notification of a time synchronization event");
}

static void initialize_sntp(void)
{
    ESP_LOGI(SNTP_TAG, "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_setservername(1, "time.google.com");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    esp_sntp_init();
    sntp_initialized = true;
}

static bool obtain_time(void)
{
    initialize_sntp();
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
    memset(&timeinfo, 0, sizeof(struct tm));
    int retry = 0;
    const int retry_count = 20;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry <= retry_count)
    {
        ESP_LOGI(SNTP_TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        ESP_ERROR_CHECK(esp_task_wdt_reset());
    }
    time(&now);
    localtime_r(&now, &timeinfo);
    ESP_ERROR_CHECK(esp_task_wdt_delete(NULL));
    return timeinfo.tm_year > (2016 - 1900);
}

static void opc_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    if (sntp_initialized != true)
    {
        if (timeinfo.tm_year < (2016 - 1900))
        {
            ESP_LOGI(SNTP_TAG, "Time is not set yet. Setting up network connection and getting time over NTP.");
            if (!obtain_time())
            {
                ESP_LOGE(SNTP_TAG, "Could not get time from NTP. Using default timestamp.");
            }
            time(&now);
        }
        localtime_r(&now, &timeinfo);
        ESP_LOGI(SNTP_TAG, "Current time: %d-%02d-%02d %02d:%02d:%02d",
                 timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                 timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    }

    if (!isServerCreated)
    {
        xTaskCreatePinnedToCore(opcua_task, "opcua_task", 16384, NULL, 6, NULL, 1);
        ESP_LOGI(MEMORY_TAG, "Heap size after OPC UA Task : %d", esp_get_free_heap_size());
        isServerCreated = true;
    }
}

static void disconnect_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
}

static void connection_scan(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, GOT_IP_EVENT, &opc_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(BASE_IP_EVENT, DISCONNECT_EVENT, &disconnect_handler, NULL));
    ESP_ERROR_CHECK(example_connect());
}

void app_main(void)
{
    ++boot_count;
    nvs_flash_init();
    if (esp_flash_encryption_enabled())
    {
        esp_flash_write_protect_crypt_cnt();
    }

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    connection_scan();

}