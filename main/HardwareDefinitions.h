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
#define LED_R GPIO_NUM_21
#define LED_G GPIO_NUM_22
#define LED_B GPIO_NUM_23
#define BTN_1 GPIO_NUM_32
#define BTN_2 GPIO_NUM_33
#define BTN_3 GPIO_NUM_34
#define GPIO_OUTPUT_PIN_SEL ( (1ULL<<LED_R) | (1ULL<<LED_G) | (1ULL<<LED_B) )
#define GPIO_INPUT_PIN_SEL  ( (1ULL<<BTN_1) | (1ULL<<BTN_2) | (1ULL<<BTN_3) )



#endif /* MAIN_HARDWAREDEFINITIONS_H_ */
