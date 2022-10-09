# Edit following two lines to set component requirements (see docs)
#set(COMPONENT_REQUIRES )
#set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS "main.c")
#set(COMPONENT_ADD_INCLUDEDIRS "")
set( COMPONENTS, bootloader driver esp32 
	espcoredump esp_event esp_ringbuf esptool_py heap
	freertos jsmn json log micro-ecc newlib nvs_flash
	pthread soc spiffs spi_flash ulp unity wpa_supplicant
	app_trace xtensa-debug-module  
	vfs app_update  main partition_table efuse
	bootloader_support esp_common xtensa esp_rom esp_timer
	   )
register_component()
