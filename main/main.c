#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#include "dht11.h"
#include "wifi.h"
#include "mqtt.h"
#include "gpio.h"

xSemaphoreHandle conexaoWifiSemaphore;
xSemaphoreHandle conexaoMQTTSemaphore;

void conectadoWifi(void * params)
{
  while(true)
  {
    if(xSemaphoreTake(conexaoWifiSemaphore, portMAX_DELAY))
    {
      // Processamento Internet
      mqtt_start();
    }
  }
}

void trataComunicacaoComServidor(void * params)
{
  struct dht11_reading read;
  char temperature[25];
  char humidity[25];
  char temp_topic[50];
  char hum_topic[50];

  if(xSemaphoreTake(conexaoMQTTSemaphore, portMAX_DELAY))
  {
    DHT11_init(GPIO_NUM_4);
    while(1) {
        do {
            read = DHT11_read();
        } while(read.status != 0);

        sprintf(temp_topic, "fse2020/180033646/%s/temperatura", room);
        sprintf(hum_topic, "fse2020/180033646/%s/umidade", room);
        sprintf(temperature, "{\"temperature\": %d}", read.temperature);
        sprintf(humidity, "{\"humidity\": %d}", read.humidity);

        mqtt_envia_mensagem(temp_topic, temperature);
        mqtt_envia_mensagem(hum_topic, humidity);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
  }
}

void app_main(void)
{
    // Inicializa o NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    conexaoWifiSemaphore = xSemaphoreCreateBinary();
    conexaoMQTTSemaphore = xSemaphoreCreateBinary();
    set_up_gpio();
    wifi_start();

    xTaskCreate(&conectadoWifi,  "Conexão ao MQTT", 4096, NULL, 1, NULL);
    xTaskCreate(&trataComunicacaoComServidor, "Comunicação com Broker", 4096, NULL, 1, NULL);
}
