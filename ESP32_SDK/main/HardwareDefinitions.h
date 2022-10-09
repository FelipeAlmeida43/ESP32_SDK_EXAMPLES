/*
 * HardwareDefinitions.h
 *
 *  Created on: 31 de mar. de 2022
 *      Author: Felipe
 */

#ifndef MAIN_HARDWAREDEFINITIONS_H_
#define MAIN_HARDWAREDEFINITIONS_H_

#include "driver/gpio.h"
#define ESP_INTR_FLAG_DEFAULT 0
#define LED_STATUS 21
#define LED_R 33
#define LED_G 34
#define LED_B 35
#define BTN_1 36
#define BTN_2 37
#define BTN_3 38
#define GPIO_OUTPUT_PIN_SEL ( (1ULL<<LED_R) | (1ULL<<LED_G) | (1ULL<<LED_B) )
#define GPIO_INPUT_PIN_SEL  ( (1ULL<<BTN_1) | (1ULL<<BTN_2) | (1ULL<<BTN_3) )



#endif /* MAIN_HARDWAREDEFINITIONS_H_ */
