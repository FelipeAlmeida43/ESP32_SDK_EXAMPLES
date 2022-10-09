#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "HardwareDefinitions.h"
static const char* TAG = "Blink";
volatile int cnt = 0;
char flagLed = '0';
int statusLed[2] = {0,0};
void blink_task(void *pvParameter);
static void IRAM_ATTR gpio_isr_handler(void* arg);
static void IRAM_ATTR gpio_isr_handler(void* arg)
{
	if(BTN_1 == (uint32_t)arg)
	{
		if( gpio_get_level( (uint32_t)arg) == 0 )
		{
			//gpio_set_level(LED_R,1);
			statusLed[0] = ! statusLed[0];
			flagLed = '1';
		}
	}
	if(BTN_2 == (uint32_t)arg)
		{
			if( gpio_get_level( (uint32_t)arg) == 0 )
			{
				//gpio_set_level(LED_R,0);
				statusLed[1] = ! statusLed[1];
				flagLed = '0';
			}
		}

}
void init_hardware()
{
	gpio_set_direction(LEDSTATUS, GPIO_MODE_OUTPUT);
	gpio_config_t io_config={
		// Descriptor Variable of GPIO driver
		.intr_type = GPIO_PIN_INTR_DISABLE,//Disable interrupt resource
		.mode = GPIO_MODE_OUTPUT,
		.pin_bit_mask = GPIO_OUTPUT_PIN_SEL,

		};gpio_config(&io_config);
		/*****************************************************************/
		gpio_config_t io_config2={
		.intr_type = GPIO_INTR_NEGEDGE,//Enable interrupt resource
		.mode = GPIO_MODE_INPUT,
		.pin_bit_mask = GPIO_INPUT_PIN_SEL,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		};gpio_config(&io_config2);
		gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
		gpio_isr_handler_add(BTN_1, gpio_isr_handler, (void*) BTN_1);
		gpio_isr_handler_add(BTN_2, gpio_isr_handler, (void*) BTN_2);
		gpio_isr_handler_add(BTN_3, gpio_isr_handler, (void*) BTN_3);
		ESP_LOGI(TAG,"HW Configured");
}
void blink_task(void *pvParameter)
{




	while(1){


		for(int i =0; i<2;i++)
		{
			switch(statusLed[i])
			{
				case 0: if(statusLed[0] == 0)
						{
							gpio_set_level(LED_G,0);
							gpio_set_level(LED_R,0);
						} else gpio_set_level(LED_G,1); gpio_set_level(LED_R,0);
				break;

				case 1:  if(statusLed[1] == 0)
						{
							gpio_set_level(LED_R,0);
							gpio_set_level(LED_G,0);
						} else gpio_set_level(LED_R,1); gpio_set_level(LED_G,0);
				break;


				default: gpio_set_level(LED_G,0);gpio_set_level(LED_R,0);
				break;
			}
		}
		vTaskDelay(100/portTICK_PERIOD_MS);
				gpio_set_level(LEDSTATUS,cnt);
				cnt = !cnt;
	}
}

void app_main(void)
{
	init_hardware();
	xTaskCreate(blink_task,"blink_task",2048,NULL,2,NULL);
	ESP_LOGI(TAG,"Blink Started");
	gpio_set_level(LED_R,0);
	gpio_set_level(LED_G,0);
	gpio_set_level(LED_B,0);


}

