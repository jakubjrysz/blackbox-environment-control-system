#ifndef SENSOR_UTILS_H
#define SENSOR_UTILS_H

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void bus_init(i2c_master_bus_handle_t *bus_handle);
void humidity_sensor_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *humidity_handle);
void pressure_sensor_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *pressure_handle);
void light_sensor_init(i2c_master_bus_handle_t bus_handle, i2c_master_dev_handle_t *light_handle);


float read_humidity(i2c_master_dev_handle_t humidity_handle);
float read_pressure(i2c_master_dev_handle_t pressure_handle);
float read_light(i2c_master_dev_handle_t light_handle);

#endif
