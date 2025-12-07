/*
 * Header file for the main OPC-UA ESP32 application logic.
 * Defines UART configuration, pins, and the main app function.
 */
#ifndef OPCUA_ESP32_H
#define OPCUA_ESP32_H

#include <stdio.h>
#include <sys/param.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_netif.h"
#include <esp_flash_encrypt.h>
#include <esp_task_wdt.h>
#include <esp_sntp.h>
#include "esp_event.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "nvs_flash.h"
#include "lwip/ip_addr.h"
#include "sdkconfig.h"

// Project dependencies
#include "ethernet_connect.h" // Assume this file exists
#include "open62541.h"
#include "model.h" // Includes the OPC-UA data model

/**
 * @brief Main application function, entry point.
 */
void app_main(void);

#endif // OPCUA_ESP32_H