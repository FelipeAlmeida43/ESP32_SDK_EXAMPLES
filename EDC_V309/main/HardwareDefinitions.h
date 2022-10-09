/*
 * HardwareDefinitions.h
 *
 *  Created on: 31 de mar. de 2022
 *      Author: Felipe
 */

#ifndef MAIN_HARDWAREDEFINITIONS_H_
#define MAIN_HARDWAREDEFINITIONS_H_

#include "driver/gpio.h"
#include "driver/uart.h"
#define ESP_INTR_FLAG_DEFAULT 0
#define LED_STATUS 2
#define BTN_AP 4
#define BTN_TIME_DEBOUNCE 30
#define GPIO_OUTPUT_PIN_SEL (1ULL<<LED_STATUS)
#define GPIO_INPUT_PIN_SEL  (1ULL<<BTN_AP)
#define SERIAL UART_NUM_0
#define SERIAL2 UART_NUM_2
#define BUF_SIZE (1024 * 2)
#define RD_BUF_SIZE (1024)
#define MQTT_SERVER "mqtt://mqtt.homecarbon.com.br"
#define MQTT_PORT 1883
#define MQTT_USER "mqtthc"
#define MQTT_PASS "I-X3tPsCojYD"
#define MQTT_ID "EDC"
char MAC_Address[18];
static const char * TAG = "WIFI";
const char* json =  "{\"ID\":67,\"T\":0.99,\"kWh_Total\":0.000194,\"kWh\":0.000000,\"Ia\":0.00,\"Ib\":0.00,\"Ic\":0.00,\"Va\":127.76,\"Vb\":127.48,\"Vc\":127.37,\"Fa\":59.88,\"Fb\":60.12,\"Fc\":60.09,\"St\":0.00,\"Pt\":0.00,\"Qt\":0.00,\"Sa\":0.00,\"Sb\":0.00,\"Sc\":0.00,\"Pa\":0.00,\"Pb\":0.00,\"Pc\":0.00,\"Qa\":0.00,\"Qb\":0.00,\"Qc\":0.00,\"PFa\":0.00,\"PFb\":0.00,\"PFc\":0.00,\"MAC\":\"ac:67:b2:eb:3a:d8\",\"Rele\":1}";


#endif /* MAIN_HARDWAREDEFINITIONS_H_ */
