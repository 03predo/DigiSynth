#include <stdio.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "esp_adc/adc_oneshot.h"
#include "pitch_values.h"

#define STACK_SIZE 2048

#define LED_GPIO 2
#define GATE_GPIO 15

static const char *TAG = "main";

TaskHandle_t xGateHandle;
StackType_t xGateStack[STACK_SIZE];
StaticTask_t xGateTCB;

TaskHandle_t xPitchHandle;
StackType_t xPitchStack[STACK_SIZE];
StaticTask_t xPitchTCB;

gptimer_handle_t debounce_timer = NULL;

adc_oneshot_unit_handle_t adc_handle;

void IRAM_ATTR button_isr_handler(void* arg) {
  xTaskResumeFromISR(xGateHandle);
}

void GateTask(void *parameters){
  static uint8_t state = 0xFF;
  while(1){
    vTaskSuspend(NULL);
    uint8_t status = 0;
    uint64_t count = 0;
    while(status != state){
      status = (status << 1) + gpio_get_level(GATE_GPIO);
      gptimer_start(debounce_timer);
      while(count > 1){
        gptimer_get_raw_count(debounce_timer, &count);
      }
      gptimer_stop(debounce_timer);
    }
    gpio_set_level(LED_GPIO, state >> 7);
    if(state == 0xFF){
      vTaskResume(xPitchHandle);
    }else{
      vTaskSuspend(xPitchHandle);
    }
    state ^= 0xFF;

  }
  vTaskDelete(NULL);
}

void PitchTask(void *parameters){
  int raw = 0;
  float curr_freq = 0, old_freq = 0;
  vTaskSuspend(NULL);
  while(1){
    adc_oneshot_read(adc_handle, ADC_CHANNEL_0, &raw);
    curr_freq = adc_to_freq(raw);
    if(curr_freq != old_freq){
      ESP_LOGI("pitch", "raw: %d, freq: %f", raw, curr_freq);
    }
    vTaskDelay(10/ portTICK_PERIOD_MS);
    old_freq = curr_freq;
    raw = 0;
  }
  vTaskDelete(NULL);
}

void app_main(void)
{
  gptimer_config_t debounce_timer_config = {
    .clk_src = GPTIMER_CLK_SRC_DEFAULT,
    .direction = GPTIMER_COUNT_UP,
    .resolution_hz = 1 * 1000 * 1000,
  };
  gptimer_new_timer(&debounce_timer_config, &debounce_timer);
  gptimer_enable(debounce_timer);

  gpio_reset_pin(GATE_GPIO);
  gpio_set_direction(GATE_GPIO, GPIO_MODE_INPUT);
  gpio_set_pull_mode(GATE_GPIO, GPIO_PULLDOWN_ONLY);
  gpio_reset_pin(LED_GPIO);
  gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
  gpio_set_level(LED_GPIO, 0);

  gpio_set_intr_type(GATE_GPIO, GPIO_INTR_ANYEDGE);
  gpio_install_isr_service(0);
  gpio_isr_handler_add(GATE_GPIO, button_isr_handler, NULL);

  
  adc_oneshot_unit_init_cfg_t unit_config = {
    .unit_id = ADC_UNIT_2,
  };
  adc_oneshot_new_unit(&unit_config, &adc_handle);
  adc_oneshot_chan_cfg_t chan_config = {
    .bitwidth = ADC_BITWIDTH_DEFAULT,
    .atten = ADC_ATTEN_DB_11,
  };
  adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_0, &chan_config);

  xGateHandle = xTaskCreateStatic(GateTask, "xGate", STACK_SIZE, (void*)1, 1, xGateStack, &xGateTCB);
  xPitchHandle = xTaskCreateStatic(PitchTask, "xPitch", STACK_SIZE, (void*)1, 1, xPitchStack, &xPitchTCB);
}
