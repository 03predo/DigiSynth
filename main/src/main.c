#include <stdio.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "midi.h"
#include "osc.h"
#include "out.h"
#include "lifo.h"

static const char *TAG = "main";

void app_main(void) {
  esp_log_level_set("midi", ESP_LOG_DEBUG);
  //esp_log_level_set("out", ESP_LOG_DEBUG);

  init_midi();
  init_output();

  while(1){
    
    ESP_LOGI(TAG, "in main loop");
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}
