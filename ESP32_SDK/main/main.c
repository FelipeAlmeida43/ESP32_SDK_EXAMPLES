#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "HardwareDefinitions.h"

static const char* TAG = "Blink";
volatile int cnt = 0;
int statusLed[2] = {0,0};
static void IRAM_ATTR gpio_isr_handler(void* arg)
{
	if(BTN_1 == (uint32_t)arg)
	{
		if( gpio_get_level( (uint32_t)arg) == 0){


			cnt+=1;
			statusLed[0] = ! statusLed[0];
		}

	}
	if(BTN_2 == (uint32_t)arg)
		{
			if( gpio_get_level( (uint32_t)arg) == 0){

				statusLed[1] = ! statusLed[1];
				cnt = 3;
			}

		}

}
void blink_task(void *pvParameter){
	int oldstate = 0;
	while(1)
	{
		vTaskDelay(100/portTICK_PERIOD_MS);
		gpio_set_level(LED_STATUS,oldstate);
		oldstate =! oldstate;
	}
}
void init_hardware()
{

		gpio_set_direction(LED_STATUS,GPIO_MODE_OUTPUT);
		gpio_config_t io_config; // Descriptor Variable of GPIO driver
		io_config.intr_type = GPIO_PIN_INTR_DISABLE;//Disable interrupt resource
		io_config.mode = GPIO_MODE_OUTPUT;
		io_config.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
		gpio_config(&io_config);
		/*****************************************************************/
		io_config.intr_type = GPIO_INTR_NEGEDGE;//Disable interrupt resource
		io_config.mode = GPIO_MODE_INPUT;
		io_config.pin_bit_mask = GPIO_INPUT_PIN_SEL;
		io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
		io_config.pull_up_en = GPIO_PULLUP_DISABLE;
		gpio_config(&io_config);
		gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
		gpio_isr_handler_add(BTN_1, gpio_isr_handler, (void*) BTN_1);
		gpio_isr_handler_add(BTN_2, gpio_isr_handler, (void*) BTN_2);
}
void checkbuttons(void *pvParameter)
{




	while(1){


		if(statusLed[0] == 0)
		{
			gpio_set_level(LED_R,0);
		}else
			{
				gpio_set_level(LED_R,1);
				gpio_set_level(LED_G,0);
				statusLed[1] = 0;
			}
		if(statusLed[1] == 0)
		{
			gpio_set_level(LED_G,0);
		}else{
				gpio_set_level(LED_G,1);
				gpio_set_level(LED_R,0);
				statusLed[0] = 0;
			}
		vTaskDelay(10/portTICK_PERIOD_MS);


	}
}

void app_main(void)
{
	init_hardware();
	xTaskCreate(blink_task,"blink_task",2048,NULL,1,NULL);
	xTaskCreate(checkbuttons,"checkbuttons",2048,NULL,1,NULL);
	//xTaskCreate(blink_task2,"blink_task2",2048,NULL,1,NULL);
	ESP_LOGI(TAG,"Blink Started");
	gpio_set_level(LED_R,0);
	gpio_set_level(LED_G,0);
	gpio_set_level(LED_B,0);


}

