#include "pcm5102a.h"

#define BLOCK_SIZE 1024

Oscillator osc;

i2s_chan_handle_t pcm5102a_handle;

static TaskHandle_t xPcm5102aTxHandle;
static StackType_t xPcm5102aTxStack[PCM5102A_TX_STACK_SIZE];
static StaticTask_t xPcm5102aTxTCB;

static TaskHandle_t xBlockGenHandle;
static StackType_t xBlockGenStack[PCM5102A_TX_STACK_SIZE];
static StaticTask_t xBlockGenTCB;

static const char *TAG = "pcm5102a";

static int16_t gen_buf[BLOCK_SIZE] = { 0 };
static int16_t tx_buf[BLOCK_SIZE] = { 0 };

static SemaphoreHandle_t xGenHandle;
static StaticSemaphore_t xGenSCB;

static SemaphoreHandle_t xCpyHandle;
static StaticSemaphore_t xCpySCB;

void init_i2s(void){
  i2s_chan_config_t chan_cfg = {
    .id = I2S_NUM_AUTO,
    .role = I2S_ROLE_MASTER,
    .dma_desc_num = 6,      // number of buffers being used
    .dma_frame_num = 2040,  // dma_frame_num <= ((4092 * 8) / (slot_num * data_bit_width)), rounded down to avoid byte loss
    .auto_clear = true,
  };

  i2s_new_channel(&chan_cfg, &pcm5102a_handle, NULL);

  i2s_std_config_t std_cfg = {
    .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
    .slot_cfg = {
      .data_bit_width = 16,
      .slot_bit_width = 32,
      .slot_mode = I2S_SLOT_MODE_MONO,
      .slot_mask = I2S_STD_SLOT_LEFT,
      .ws_width = 16,
      .ws_pol = false,
      .bit_shift = true,
      .msb_right = true,
    },
    .gpio_cfg = {
      .mclk = I2S_GPIO_UNUSED,
      .bclk = PCM5102A_BCK,
      .ws = PCM5102A_WS,
      .dout = PCM5102A_DOUT,
      .din = I2S_GPIO_UNUSED,
      .invert_flags = {
        .mclk_inv = false,
        .bclk_inv = false,
        .ws_inv = false,
      },
    },
  };

  i2s_channel_init_std_mode(pcm5102a_handle, &std_cfg);

  i2s_channel_enable(pcm5102a_handle);
}

void BlockGenTask(void *parameters){
  while(1){
    for(int i = 0; i < BLOCK_SIZE; ++i){
      gen_buf[i] = next_sample_16bit(&osc);
    }
    xSemaphoreGive(xGenHandle);
    xSemaphoreTake(xCpyHandle, portMAX_DELAY);
  }
  vTaskDelete(NULL);
}

void Pcm5102aTxTask(void *parameters){
  size_t bytes_written = 0;
  while(1){
    xSemaphoreTake(xGenHandle, portMAX_DELAY);
    memcpy(tx_buf, gen_buf, BLOCK_SIZE * 2);
    xSemaphoreGive(xCpyHandle);
    i2s_channel_write(pcm5102a_handle, tx_buf, BLOCK_SIZE * 2, &bytes_written, 1000 / portTICK_PERIOD_MS);
    if(bytes_written != BLOCK_SIZE * 2){
      ESP_LOGI(TAG, "bytes_written=%d", bytes_written);
    }
  }
  i2s_channel_disable(pcm5102a_handle);
  i2s_del_channel(pcm5102a_handle);
  vTaskDelete(NULL);
}

void init_pcm5102a(void){  
  init_oscillator(&osc, &sin_wave, 0.50, 440);
  init_i2s();
  xGenHandle = xSemaphoreCreateBinaryStatic(&xGenSCB);
  xCpyHandle = xSemaphoreCreateBinaryStatic(&xCpySCB);
  xBlockGenHandle = xTaskCreateStatic(BlockGenTask, "xPcm5102aTx", PCM5102A_TX_STACK_SIZE, (void*)0, 1, xBlockGenStack, &xBlockGenTCB);
  xPcm5102aTxHandle = xTaskCreateStatic(Pcm5102aTxTask, "xPcm5102aTx", PCM5102A_TX_STACK_SIZE, (void*)0, 1, xPcm5102aTxStack, &xPcm5102aTxTCB);
  ESP_LOGI(TAG, "pcm5102a initialized");
}
