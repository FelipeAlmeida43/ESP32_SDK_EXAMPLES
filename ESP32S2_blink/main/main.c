#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "driver/gpio.h"
static const char* TAG = "Blink";
#define LED_R 21
#define LED_G 33
#define LED_B 2
#define GPIO_OUTPUT_PIN_SEL ( (1ULL<<LED_R) | (1ULL<<LED_G) | (1ULL<<LED_B) )
esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}
void blink_task(void *pvParameter){
	 gpio_config_t io_config={
	    	// Descriptor Variable of GPIO driver
	    	.intr_type = 0,//Disable interrupt resource
	    	.mode = GPIO_MODE_OUTPUT,
	    	.pin_bit_mask = GPIO_OUTPUT_PIN_SEL,

	    	};gpio_config(&io_config);
	 int level = 0;
	    while (true) {
	        gpio_set_level(LED_G, level);
	        level = !level;
	        vTaskDelay(100 / portTICK_PERIOD_MS);
	    }
}
void blink_task_(void *pvParameter){
	 gpio_config_t io_config={
	    	// Descriptor Variable of GPIO driver
	    	.intr_type = 0,//Disable interrupt resource
	    	.mode = GPIO_MODE_OUTPUT,
	    	.pin_bit_mask = GPIO_OUTPUT_PIN_SEL,

	    	};gpio_config(&io_config);
	 int level = 0;
	    while (true) {
	        gpio_set_level(LED_R, level);
	        level = !level;
	        vTaskDelay(500 / portTICK_PERIOD_MS);
	    }
}
void app_main(void)
{
    nvs_flash_init();
    ESP_LOGI(TAG,"HW Configured");
    xTaskCreate(blink_task,"blink_task",2048,NULL,2,NULL);
    xTaskCreate(blink_task_,"blink_task_",2048,NULL,2,NULL);


    //gpio_set_direction(21, GPIO_MODE_OUTPUT);

}

;
