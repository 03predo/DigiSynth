#include <stdio.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "midi.h"
#include "osc.h"
#include "pcm5102a.h"

static const char *TAG = "main";

void app_main(void) {
  // component log levels
  //esp_log_level_set("midi", ESP_LOG_DEBUG);
  //esp_log_level_set("pcm5102a", ESP_LOG_DEBUG);

  init_midi();
  init_pcm5102a();

  while(1){
    ESP_LOGI(TAG, "in main loop");
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}
