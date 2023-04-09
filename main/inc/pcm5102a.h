#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/i2s_std.h"

#include "osc.h"

#pragma once

#define PCM5102A_DOUT GPIO_NUM_33
#define PCM5102A_BCK GPIO_NUM_27
#define PCM5102A_WS GPIO_NUM_32
#define PCM5102A_SAMPLE_RATE SAMPLE_RATE

extern TaskHandle_t xPcm5102aTxHandle;

void init_pcm5102a(void);
