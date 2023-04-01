#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/i2s_std.h"

#include "osc.h"
#include "midi.h"

#pragma once

#define PCM5102A_DOUT GPIO_NUM_27
#define PCM5102A_BCK GPIO_NUM_32
#define PCM5102A_WS GPIO_NUM_33
#define PCM5102A_BITWIDTH I2S_DATA_BIT_WIDTH_16BIT
#define PCM5102A_TX_STACK_SIZE 8192

void init_pcm5102a(void);
