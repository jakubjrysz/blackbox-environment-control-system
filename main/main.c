#include <stdio.h>
#include <math.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "sensor_utils.h"

static const char *TAG = "OPC_UA_WEATHER";

void app_main(void)
{
    uint8_t data[2];
    i2c_master_bus_handle_t bus_handle;
    i2c_master_dev_handle_t light_handle, humidity_handle, pressure_handle;

    bus_init(&bus_handle);
    light_sensor_init(bus_handle, &light_handle);
    humidity_sensor_init(&bus_handle, &humidity_handle);
    pressure_sensor_init(&bus_handle, &pressure_handle);

    while (1) {
        printf("Humidity: %.2f %% \n", read_humidity(humidity_handle));
        printf("Pressure: %.2f hPa \n", read_pressure(pressure_handle));
        printf("Light: %.2f lux \n", read_light(light_handle));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
