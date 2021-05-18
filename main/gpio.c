#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "gpio.h"
#include "mqtt.h"

xQueueHandle input_queue;

static void IRAM_ATTR gpio_isr_handler(void *args)
{
  int pin = (int)args;
  xQueueSendFromISR(input_queue, &pin, NULL);
}

void input_handler(void *params)
{
  while(true)
  {
    int pin;
    if(xQueueReceive(input_queue, &pin, portMAX_DELAY))
    {
      // De-bouncing
      char topic[50];
      int state = gpio_get_level(pin);

      if(state == 1)
      {
        gpio_isr_handler_remove(pin);

        printf("apertei\n");
        sprintf(topic, "fse2020/180033646/%s/estado", room);

        mqtt_send_message(topic, "{\"input\": 1}");

        // Habilita novamente a interrupção
        vTaskDelay(50 / portTICK_PERIOD_MS);
        gpio_isr_handler_add(pin, gpio_isr_handler, NULL);
      }

    }
  }
}


void set_up_gpio() {
    gpio_pad_select_gpio(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);

    gpio_pad_select_gpio(BUTTON);
    gpio_set_direction(BUTTON, GPIO_MODE_INPUT);
    gpio_pulldown_en(BUTTON);
    gpio_pullup_dis(BUTTON);
    gpio_set_intr_type(BUTTON, GPIO_INTR_POSEDGE);

    input_queue = xQueueCreate(10, sizeof(int));
    xTaskCreate(input_handler, "InputHandler", 2048, NULL, 1, NULL);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON, gpio_isr_handler, (void *) BUTTON);
}

void turn_on_led() {
    gpio_set_level(LED, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);
}

void turn_off_led() {
    gpio_set_level(LED, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
}
