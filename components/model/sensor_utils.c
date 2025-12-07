#include "sensor_utils.h"
#include "esp_log.h"
#include <math.h>
#include "driver/i2c_master.h"

#define I2C_MASTER_SCL_IO           18                          /* GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           21                          /* GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              I2C_NUM_0                   /* I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ          100000                      /* I2C master clock frequency */
#define I2C_MASTER_TIMEOUT_MS       1000
#define I2C_MASTER_TIMEOUT_MS       1000

#define HUMIDITY_SENSOR_ADDR 0x5F
#define PRESSURE_SENSOR_ADDR 0x5D
#define LIGHT_SENSOR_ADDR 0x39

void bus_init(i2c_master_bus_handle_t *bus_handle){
/*
    main i2c bus setup
*/
    ESP_LOGI("I2C bus", "BUS initialization started \n");

    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, bus_handle));
}

void humidity_sensor_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *humidity_handle)
{
    ESP_LOGI("HTS221", "Humidity sensor initialization started");

    i2c_device_config_t cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = HUMIDITY_SENSOR_ADDR,
        .scl_speed_hz = 100000,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &cfg, humidity_handle));

    uint8_t buf[2];
    uint8_t id;
    uint8_t id_reg = 0x0F;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(*humidity_handle, &id_reg, 1, &id, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
    ESP_LOGI("HTS221", "WHO_AM_I = 0x%X", id);

    buf[0] = 0x10; buf[1] = 0x1B; ESP_ERROR_CHECK(i2c_master_transmit(*humidity_handle, buf, 2, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
    buf[0] = 0x20; buf[1] = 0x85; ESP_ERROR_CHECK(i2c_master_transmit(*humidity_handle, buf, 2, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
    buf[0] = 0x21; buf[1] = 0x00; ESP_ERROR_CHECK(i2c_master_transmit(*humidity_handle, buf, 2, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
    buf[0] = 0x22; buf[1] = 0x00; ESP_ERROR_CHECK(i2c_master_transmit(*humidity_handle, buf, 2, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
}

void pressure_sensor_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *pressure_handle)
{
    ESP_LOGI("LPS331", "Pressure sensor initialization started");

    i2c_device_config_t cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = PRESSURE_SENSOR_ADDR,
        .scl_speed_hz = 100000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &cfg, pressure_handle));

    uint8_t buf[2];
    uint8_t id;
    uint8_t id_reg = 0x0F;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(*pressure_handle, &id_reg, 1, &id, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
    ESP_LOGI("LPS331", "WHO_AM_I = 0x%X", id);

    uint8_t write_addr[] = {0x10, 0x20, 0x21, 0x22};
    uint8_t write_val[] = {0x00, 0xE4, 0x80, 0x00};
    for(int i = 0; i < 4; i++){
        buf[0] = write_addr[i]; buf[1] = write_val[i];
        ESP_ERROR_CHECK(i2c_master_transmit(*pressure_handle, buf, 2, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
    }
}

void light_sensor_init(i2c_master_bus_handle_t bus_handle, i2c_master_dev_handle_t *light_handle)
{
    ESP_LOGI("TSL2571", "Light sensor initialization started");

    i2c_device_config_t cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = LIGHT_SENSOR_ADDR,
        .scl_speed_hz = 100000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &cfg, light_handle));

    uint8_t write_addr[] = {0x00|0x80,0x01|0x80,0x03|0x80,0x0D|0x80,0x0F|0x80};
    uint8_t write_val[] = {0x03,0xED,0xB6,0x00,0x00};
    uint8_t buf[2];
    uint8_t id_reg = 0x12|0x80;
    uint8_t id;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(*light_handle, &id_reg, 1, &id, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
    for(int i=0;i<5;i++){ buf[0]=write_addr[i]; buf[1]=write_val[i]; ESP_ERROR_CHECK(i2c_master_transmit(*light_handle, buf, 2, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS)); }
}

float read_humidity(i2c_master_dev_handle_t humidity_handle){
    ESP_LOGI("HTS221", "humidity read called\n");
    uint8_t h0_rh_x2, h1_rh_x2;
    uint8_t h0_out_l, h0_out_h, h1_out_l, h1_out_h, h_out_l, h_out_h;

    uint8_t reg;

    reg = 0x30;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(humidity_handle, &reg, 1, &h0_rh_x2, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
    ESP_LOGI("HTS221", "Read h0_rh_x2 register: %d \n", h0_rh_x2);

    reg = 0x31;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(humidity_handle, &reg, 1, &h1_rh_x2, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
    ESP_LOGI("HTS221", "Read h1_rh_x2 register: %d \n", h1_rh_x2);

    reg = 0x36;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(humidity_handle, &reg, 1, &h0_out_l, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
    ESP_LOGI("HTS221", "Read h0_out_l register: %d \n", h0_out_l);

    reg = 0x37;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(humidity_handle, &reg, 1, &h0_out_h, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
    ESP_LOGI("HTS221", "Read h0_out_h register: %d \n", h0_out_h);

    reg = 0x3A;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(humidity_handle, &reg, 1, &h1_out_l, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
    ESP_LOGI("HTS221", "Read h1_out_l register: %d \n", h1_out_l);

    reg = 0x3B;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(humidity_handle, &reg, 1, &h1_out_h, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
    ESP_LOGI("HTS221", "Read h1_out_h register: %d \n", h1_out_h);

    reg = 0x28;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(humidity_handle, &reg, 1, &h_out_l, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
    ESP_LOGI("HTS221", "Read h_out_l register: %d \n", h_out_l);

    reg = 0x29;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(humidity_handle, &reg, 1, &h_out_h, 1, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
    ESP_LOGI("HTS221", "Read h_out_h register: %d \n", h_out_h);

    float H0_rh = h0_rh_x2 / 2.0f;
    float H1_rh = h1_rh_x2 / 2.0f;

    int16_t H0_T0_out = ((int16_t)h0_out_h << 8) | h0_out_l;
    int16_t H1_T0_out = ((int16_t)h1_out_h << 8) | h1_out_l;
    int16_t H_T_out = ((int16_t)h_out_h << 8) | h_out_l;

    float humidity = (((float)(H_T_out - H0_T0_out)) * (H1_rh - H0_rh)) /
                     (float)(H1_T0_out - H0_T0_out) +
                     H0_rh;

    if (humidity > 100.0f) humidity = 100.0f;
    if (humidity < 0.0f) humidity = 0.0f;

    return humidity;
}
float read_temp_pressure(i2c_master_dev_handle_t pressure_handle){
    const uint8_t pressure_data_address = 0x28 | 0x80;
    const uint8_t temp_data_address = 0x2B | 0x80;
    uint8_t data_buffer[3]; 
    
    ESP_ERROR_CHECK(i2c_master_transmit_receive(pressure_handle, &pressure_data_address, 1, data_buffer, 3, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
    uint32_t raw_pressure = ((uint32_t)data_buffer[2] << 16) | ((uint32_t)data_buffer[1] << 8) | data_buffer[0];
    float pressure_hPa = raw_pressure / 4096.0f;

    ESP_ERROR_CHECK(i2c_master_transmit_receive(pressure_handle, &temp_data_address, 1, data_buffer, 2, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
    int16_t raw_temp = (int16_t)((data_buffer[1] << 8) | data_buffer[0]);
    float temp = 42.5f + (raw_temp / 480.0f);

    //ESP_LOGI("LPS331", "TEMP FROM PRESSURE SENSOR: %.2f °C", temp);
    //ESP_LOGI("LPS331", "PRESSURE: %.2f hPa", pressure_hPa);
    
    return temp;
}
float read_light(i2c_master_dev_handle_t light_handle){
    uint8_t buffer[4];
    uint8_t start_address = 0x14 | 0xA0;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(light_handle, &start_address, 4, buffer, 4, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS));
    uint16_t c0 = ((uint16_t)buffer[1] << 8) | buffer[0];
    uint16_t c1 = ((uint16_t)buffer[3] << 8) | buffer[2];

    float ATIME_ms = 50.0f;
    float AGAINx = 1.0f;
    float GA = 1.0f;

    float CPL = (ATIME_ms * AGAINx) / (GA * 60.0f);

    float Lux1 = (1.0f * c0 - 1.87f * c1) / CPL;
    float Lux2 = (0.63f * c0 - 1.0f * c1) / CPL;

    float lux = fmaxf(fmaxf(Lux1, Lux2), 0.0f);
    //ESP_LOGI("TSL2571", "LIGHT: %.2f lux", lux);
    return lux; 
}
