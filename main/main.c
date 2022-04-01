#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "HardwareDefinitions.h"
static const char* TAG = "Blink";
volatile int cnt = 0;
static void IRAM_ATTR gpio_isr_handler(void* arg)
{
	if(BTN_1 == (uint32_t)arg)
	{
		if( gpio_get_level( (uint32_t)arg) == 0){
			gpio_set_level(LED_R,1);
		}

	}else gpio_set_level(LED_R,0);

}
void blink_task(void *pvParameter)
{
	//gpio_pad_select_gpio(BTN_1);
	//gpio_set_direction(BTN_1,GPIO_MODE_INPUT);
	//gpio_set_pull_mode(BTN_1, GPIO_PULLUP_ONLY);
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
	io_config.pull_up_en = GPIO_PULLUP_ENABLE;
	gpio_config(&io_config);
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	gpio_isr_handler_add(BTN_1, gpio_isr_handler, (void*) BTN_1);
	int count = 0,
		oldstate = 0;

	while(1){

		vTaskDelay(300/portTICK_PERIOD_MS);

	}
}

void app_main(void)
{
	xTaskCreate(blink_task,"blink_task",2048,NULL,1,NULL);
	ESP_LOGI(TAG,"Blink Started");
	gpio_set_level(LED_R,0);
	gpio_set_level(LED_G,0);

	gpio_set_level(LED_B,0);


}

