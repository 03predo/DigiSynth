#include <stdio.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2s_std.h"
#include "driver/gpio.h"

#include "midi.h"

#include <math.h>
#include <stdint.h>

static const char *TAG = "main";

void app_main(void) {
  // component log levels
  esp_log_level_set("midi", ESP_LOG_DEBUG);

  init_midi();
  while(1){
    ESP_LOGI(TAG, "hello world");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
