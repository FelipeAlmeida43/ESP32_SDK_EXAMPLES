#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_http_client.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "string.h"
#include "HardwareDefinitions.h"
#include "stdint.h"
#include "string.h"
//#include "Wifi.h"
#define SSID "MeshCasa"
#define PASS "felipeepamela1101"
#define U2RXD 17
#define U2TXD 16

char byte;
int mac[6];
int statusLed = 0;
int Data_length = 0;
int UART1_data_length = 0;
char rxbuf[500];     //Buffer di ricezione
char buffer_json[500];
uint16_t rx_fifo_len;        //Lunghezza dati
bool flagRxFinish = false;
//string Buffer ="";
//String Buffer2;
static QueueHandle_t uart1_queue;
static QueueHandle_t uart2_queue;
uart_event_t event;
bool lastStateBTN , actualStateBTN = false;
bool rxFinish = false;
esp_mqtt_client_handle_t client;
void getMacAddress()
{
  //WiFi.macAddress(mac);
  sprintf(MAC_Address, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}
static void IRAM_ATTR gpio_isr_handler(void* arg)
{
	if(BTN_AP == (uint32_t)arg)
	{
		if( gpio_get_level( (uint32_t)arg) == 0)
		{
			statusLed +=1 ;
			lastStateBTN = false;
			uart_write_bytes(SERIAL,"BTN_AP\r\n", sizeof("BTN_AP\r\n"));
            uart_write_bytes(SERIAL2, "WIFI-NEW\r\n", sizeof("WIFI-NEW\r\n"));

		}
	}
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, "edc/100004084/relay", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "edc/100004084/time", 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        if(strncmp(event->data , "on", event->data_len)==0){
			uart_write_bytes(SERIAL2, "RELAY-ON\r\n", sizeof("RELAY-ON\r\n"));
			uart_write_bytes(SERIAL, "RELAY-ON\r\n", sizeof("RELAY-ON\r\n"));
		} 
        if(strncmp(event->data , "off", event->data_len)==0){
			uart_write_bytes(SERIAL2, "RELAY-OFF\r\n", sizeof("RELAY-OFF\r\n"));
			uart_write_bytes(SERIAL, "RELAY-OFF\r\n", sizeof("RELAY-OFF\r\n"));
		} 
        if (strncmp(event->data, "send binary please", event->data_len) == 0) {
            ESP_LOGI(TAG, "Sending the binary");
            //send_binary(client);
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
            ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
            ESP_LOGI(TAG, "Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
                     strerror(event->error_handle->esp_transport_sock_errno));
        } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
            ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
        } else {
            ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}
static void mqtt_app_start(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MQTT_SERVER,
		.client_id= MQTT_ID,
        .username = MQTT_USER,
		.password = MQTT_PASS
    };

    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    /*esp_mqtt_client_handle_t*/ client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}
void checkWifi(void *pvParameter)
{
	while(1){


		if(statusLed == 2)
		{
			actualStateBTN = gpio_get_level(BTN_AP);
			vTaskDelay(BTN_TIME_DEBOUNCE/portTICK_PERIOD_MS);
			if(actualStateBTN != lastStateBTN)	gpio_set_level(LED_STATUS,1);
			//statusLed= 0;
			lastStateBTN = true;
			//uart_write_bytes(SERIAL1, "BTN_AP\r\n", sizeof("BTN_AP\r\n"));

		}else if(statusLed > 2)
			{
				actualStateBTN = gpio_get_level(BTN_AP);
				vTaskDelay(BTN_TIME_DEBOUNCE/portTICK_PERIOD_MS);
				if(actualStateBTN != lastStateBTN)gpio_set_level(LED_STATUS,0);
				statusLed = 0;
				lastStateBTN = true;

			}

		vTaskDelay(100/portTICK_PERIOD_MS);


	}
	vTaskDelete(NULL);
}
void blink(void *pvParameter)
{
	int level = 0;
	while(1)
	{
		 gpio_set_level(LED_STATUS, level);
		 level = !level;
		 vTaskDelay(1000/portTICK_PERIOD_MS);
		 
		
	}

	vTaskDelete(NULL);
}
void setupMSP_ESP()
{
  //Serial.println("SETUP MSP<---->ESP");
 
  
    if (strncmp(buffer_json,"{WIFI-STATUS?}\r\n",sizeof("{WIFI-STATUS?}\r\n") ) ==0 )
    //if(counteudo.equals(MSG_CONNECTION) == 1 )
    /*if((strncmp(counteudo.c_str(),"{WIFI-STATUS?}\r\n",sizeof("{WIFI-STATUS?}\r\n") ) ==0 ) ||
        (strncmp(counteudo.c_str(),"{WIFI-STATUS?}",sizeof("{WIFI-STATUS?}") ) ==0 ))*/
    {
      
        //Serial2.print("WIFI-OK\r\n");
        for (int i = 0; i < 3; i++)uart_write_bytes(SERIAL2,"WIFI-OK\r\n",sizeof("WIFI-OK\r\n"));
        //indice = WIFI_OK;
        uart_write_bytes(SERIAL,"WIFI OK",sizeof("WIFI OK"));
        //delay(500);
      
      /*else
        (WiFi.begin(rede, senha));*/
      //delay(500);
    }
    
   
   
  
   if (strncmp(buffer_json,"{EDC-ID?}\r\n",sizeof("{EDC-ID?}\r\n") ) ==0 )
   //if(counteudo.equals(MSG_ID) == 1 )
  /*if( (strncmp(counteudo.c_str(),"{EDC-ID?}\r\n",sizeof("{EDC-ID?}\r\n") ) ==0 )||
      (strncmp(counteudo.c_str(),"{EDC-ID?}",sizeof("{EDC-ID?}") ) ==0 ) )*/
    {

      char resposta[30];
      sprintf(resposta, "EDC-ID:%d\r\n", 67);
      for (int i = 0; i < 3; i++)uart_write_bytes(SERIAL2,resposta,sizeof(resposta));
     
      //delay(500);
      uart_write_bytes(SERIAL,"EDC-ID OK",sizeof("EDC-ID OK"));
    }
    
   
 
    if (strncmp(buffer_json,"{EDC-MAC?}\r\n",sizeof("{EDC_MAC?}\r\n"))==0)
    //if(counteudo.equals(MSG_MAC) == 1 )
   /* if( (strncmp(counteudo.c_str(),"{EDC-MAC?}\r\n",sizeof("{EDC_MAC?}\r\n"))==0)||
        (strncmp(counteudo.c_str(),"{EDC-MAC?}",sizeof("{EDC_MAC?}"))==0))*/
    {
      char resposta[42];
      sprintf(resposta, "EDC-MAC:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
              mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
      for (int i = 0; i < 3; i++)uart_write_bytes(SERIAL2,resposta,sizeof(resposta));        
      //Serial2.print(resposta);
      //indice = MAC_OK;
      //delay(500);
      uart_write_bytes(SERIAL,"MAC OK",sizeof("MAC OK"));
    }
    
   /*
    if (conteudo.equals("{END-SETUP}"))

    {

     
      Serial.println("END SETUP");
      
    }
    
    if (conteudo.compareTo("\r{TIME?}") == 0)
    {
      Serial2.printf("TIME:%d\r\n", InfoHW.tempo);
      Serial.printf("Tempo:%d", InfoHW.tempo);
    }
    Serial.println("DEFAULT");
*/
    
  
}
 static void UART_ISR_ROUTINE(void *pvParameters)
{
    
    //size_t buffered_size;
    bool exit_condition = false;
    uint8_t *dtmp = (uint8_t *) malloc(BUF_SIZE);
    memset(buffer_json, 0, 500);
    //Infinite loop to run main bulk of task
    while (1) {
     
        //Loop will continually block (i.e. wait) on event messages from the event queue
        if(xQueueReceive(uart2_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
         
            //Handle received event
            if(event.type == UART_DATA)
            {
           
                uint8_t UART2_data[1024];
                int UART2_data_length = 0;
                //int UART2_data_length = 1024;
                ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_2, (size_t*)&UART2_data_length));
                UART2_data_length = uart_read_bytes(UART_NUM_2, UART2_data, UART2_data_length, 10);
                Data_length += UART2_data_length;
                
                for(int i=0; i<UART2_data_length;i++)
                {
                   
                   
                   rxbuf[i]  = UART2_data[i];
                   //Serial.print(rxbuf[i]) ;
                   if(rxbuf[i]!= '\n'){
                    //conteudo.concat(rxbuf[i]);
                    //uart_write_bytes(SERIAL, &rxbuf[i], sizeof(rxbuf[i]));
                   sprintf(&buffer_json[i],"%s",&rxbuf[i]);
                   }
                   else rxFinish = true;
                   
                   
                }
                /*for(int i=0;i<UART2_data_length;i++)    {
	                	sprintf(&buffer_json[i],"%s",&rxbuf[i]);
	                }*/
             
                
        }if(event.type == UART_BUFFER_FULL)  {

    	   //Serial.print("Buffer FULL") ;
       }
        
        //If you want to break out of the loop due to certain conditions, set exit condition to true
        if (exit_condition) {
            break;
        }
    }
    }
    //Out side of loop now. Task needs to clean up and self terminate before returning
    free(dtmp);
    dtmp = NULL;  
    vTaskDelete(NULL);
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
		gpio_isr_handler_add(BTN_AP, gpio_isr_handler, (void*) BTN_AP);
		uart_config_t Config_UART2 = {
		        .baud_rate = 9600,
		        .data_bits = UART_DATA_8_BITS,
		        .parity = UART_PARITY_DISABLE,
		        .stop_bits = UART_STOP_BITS_1,
		        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
		    };
		    uart_param_config(SERIAL2, &Config_UART2);
		    uart_set_pin(SERIAL2, U2TXD, U2RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	        uart_driver_install(SERIAL2, BUF_SIZE, BUF_SIZE, 20, &uart2_queue, 0);
			xTaskCreate(UART_ISR_ROUTINE, "UART_ISR_ROUTINE", 2048, NULL, 12, NULL);
	        uart_config_t Config_UART = {
	        		        .baud_rate = 115200,
	        		        .data_bits = UART_DATA_8_BITS,
	        		        .parity = UART_PARITY_DISABLE,
	        		        .stop_bits = UART_STOP_BITS_1,
	        		        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	        		    };
	        		    uart_param_config(SERIAL, &Config_UART);
	        		    uart_set_pin(SERIAL, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	        	        uart_driver_install(SERIAL, BUF_SIZE, BUF_SIZE, 20, &uart1_queue, 0);

}
esp_err_t _http_event_handle(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
            printf("%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                printf("%.*s", evt->data_len, (char*)evt->data);
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}
void http_client_request()
{
	/* for(int i=0;i<Data_length;i++)    {
	                	sprintf(&buffer_json[i],"%s",&rxbuf[i]);
	                }*/
	 //for(int i=0;i<Data_length;i++)uart_write_bytes(SERIAL1, &rxbuf[i], sizeof(rxbuf[i]));
	//uart_write_bytes(SERIAL1, &buffer_json, Data_length);
    esp_http_client_config_t config = {
    .url = "http://data.prod.konkerlabs.net:80/pub/jbat2lmvsuh0/1",
	.username = "jbat2lmvsuh0",
	.password = "fP5G4vJctEYN",
	.auth_type = HTTP_AUTH_TYPE_BASIC,
    .event_handler = _http_event_handle,
	.keep_alive_enable = true,
	};
   // for(int i=0;i<Data_length;i++) strncat(&buffer_json[i],&rxbuf[i],Data_length);
    //for(int i=0;i<Data_length;i++)memccpy(&buffer_json[i],&rxbuf[i],'\0',Data_length);


    esp_http_client_handle_t client = esp_http_client_init(&config);

    //const char* json =  "{\"ID\":67,\"T\":0.99,\"kWh_Total\":0.000194,\"kWh\":0.000000,\"Ia\":0.00,\"Ib\":0.00,\"Ic\":0.00,\"Va\":127.76,\"Vb\":127.48,\"Vc\":127.37,\"Fa\":59.88,\"Fb\":60.12,\"Fc\":60.09,\"St\":0.00,\"Pt\":0.00,\"Qt\":0.00,\"Sa\":0.00,\"Sb\":0.00,\"Sc\":0.00,\"Pa\":0.00,\"Pb\":0.00,\"Pc\":0.00,\"Qa\":0.00,\"Qb\":0.00,\"Qc\":0.00,\"PFa\":0.00,\"PFb\":0.00,\"PFc\":0.00,\"MAC\":\"ac:67:b2:eb:3a:d8\",\"Rele\":1}";
    esp_http_client_set_url(client, "http://data.prod.konkerlabs.net:80/pub/jbat2lmvsuh0/1");
    esp_http_client_set_method(client,HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client,buffer_json, Data_length);
    //for(int i=0;i<Data_length;i++)esp_http_client_set_post_field(client, &rxbuf[i], sizeof(rxbuf[i]));
    for(int i=0;i<Data_length;i++)rxbuf[i] = 0;
    Data_length =0;

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
    ESP_LOGI(TAG, "Status = %d, content_length = %d",
            esp_http_client_get_status_code(client),
            esp_http_client_get_content_length(client));
    }
    flagRxFinish = false;
    esp_http_client_cleanup(client);
}
void app_main(void)
{
	init_hardware();
    nvs_flash_init();
    tcpip_adapter_init();

    ESP_LOGI(TAG,"Wifi Init Config");
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
    /*ESP_LOGI(TAG,"Got IP:%s",
    	    		ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));*/
    ESP_LOGI(TAG,"Finish Init Config");
    vTaskDelay(1000/portTICK_PERIOD_MS);
    xTaskCreate(checkWifi,"checkWifi",2048,NULL,1,NULL);
    xTaskCreate(blink,"blink",2048,NULL,2,NULL);
    xTaskCreate(UART_ISR_ROUTINE, "UART_ISR_ROUTINE", 2048, NULL, 12, NULL);
    //gpio_set_direction(GPIO_NUM_21, GPIO_MODE_OUTPUT);
    mqtt_app_start();
	
    while (true) {




        vTaskDelay(500 / portTICK_PERIOD_MS);
		
       /* if(flagRxFinish){
        	for(int i=0;i<Data_length;i++)uart_write_bytes(SERIAL, &rxbuf[i], sizeof(rxbuf[i]));
        	//http_client_request();
        }*/
if(rxFinish)

  {
    setupMSP_ESP();
    char str[3];
    sprintf(str, "%d", Data_length);
    uart_write_bytes(SERIAL,"Data : ",sizeof("Data : "));
    uart_write_bytes(SERIAL,str,sizeof(str));
    //uart_write_bytes(SERIAL,"Buffer JSON : ",sizeof("Buffer JSON : "));
    //Serial.println(buffer_json); // Imprime Buffer em HEX 
    //for(int i=0;i<Data_length;i++)uart_write_bytes(SERIAL, &rxbuf[i], sizeof(rxbuf[i]));
    uart_write_bytes(SERIAL,&buffer_json,343);
   esp_mqtt_client_publish(client, "edc/100004084/json", &buffer_json, strlen(buffer_json), 0, 0);
             
     /*char buf_size[3];
     sprintf(buf_size, "%d", sizeof(buffer_json));
     uart_write_bytes(SERIAL,buf_size,strlen(buf_size));*/
     //uart_write_bytes(SERIAL,&buffer_json,strlen(buffer_json));
     
     
     //memset(rxbuf,0,sizeof(rxbuf));
     
     
     rxFinish= false;
	 Data_length = 0;
  }

    }
}

