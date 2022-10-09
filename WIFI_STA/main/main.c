#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
//extern EventGroupHandle_t wifi_event_group;
#define LED_STATUS 2


#define SSID "MeshCasa"
#define PASS "felipeepamela1101"
const char * TAG = "Wifi";
system_event_t *event;
static esp_err_t event_handler(void *ctx, system_event_t *event)
{

    return ESP_OK;
}
void blink_task()
{

	    int level = 0;
	    while (true) {
	        gpio_set_level(LED_STATUS, level);
	        level = !level;
	        vTaskDelay(5000 / portTICK_PERIOD_MS);
	    }
}
void wifi_init_station()
{

		ESP_LOGI(TAG,"Wifi Init Config");
		tcpip_adapter_init();
	    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );

	    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
	    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
	    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
	    wifi_config_t sta_config = {
	        .sta = {
	            .ssid = SSID,
	            .password = PASS,
	            .bssid_set = false
	        }
	    };
	    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
	    ESP_ERROR_CHECK( esp_wifi_start() );
	    ESP_ERROR_CHECK( esp_wifi_connect() );
	    ESP_LOGI(TAG,"Finish Init Config");

	    xTaskCreate(blink_task, "blink_task", 2048, NULL, 2, NULL);
	   /*ESP_LOGI(TAG,"Got IP:%s",
	    		ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));*/

}
void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES){
    	ESP_ERROR_CHECK(nvs_flash_erase());
    	ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    //wifi_event_group = xEventGroupCreate();
    gpio_set_direction(LED_STATUS, GPIO_MODE_OUTPUT);

    vTaskDelay(2000/portTICK_PERIOD_MS);

    wifi_init_station();
    gpio_set_level(LED_STATUS, 1);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    gpio_set_level(LED_STATUS, 0);


    while (true) {

        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
}

