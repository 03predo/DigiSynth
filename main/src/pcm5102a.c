#include "pcm5102a.h"

Oscillator osc;

i2s_chan_handle_t pcm5102a_handle;

static TaskHandle_t xPcm5102aTxHandle;
static StackType_t xPcm5102aTxStack[PCM5102A_TX_STACK_SIZE];
static StaticTask_t xPcm5102aTxTCB;

static const char *TAG = "pcm5102a";

#define BLOCK_SIZE 2048

static int32_t src_buf[BLOCK_SIZE] ={ 0 };
size_t bytes_written = 0;

static IRAM_ATTR bool i2s_tx_callback(i2s_chan_handle_t handle, i2s_event_data_t *event, void *user_ctx)
{
  xTaskResumeFromISR(xPcm5102aTxHandle);
  return false;
}

void init_i2s(void){
  i2s_chan_config_t chang_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);

  i2s_new_channel(&chang_cfg, &pcm5102a_handle, NULL);

  i2s_std_config_t std_cfg = {
    .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(48000),
    .slot_cfg = {
      .data_bit_width = 32,
      .slot_bit_width = 64,
      .slot_mode = I2S_SLOT_MODE_MONO,
      .slot_mask = I2S_STD_SLOT_LEFT,
      .ws_width = 32,
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

  i2s_event_callbacks_t cbs = {
    .on_recv = NULL,
    .on_recv_q_ovf = NULL,
    .on_sent = i2s_tx_callback,
    .on_send_q_ovf = NULL,
  };
  i2s_channel_register_event_callback(pcm5102a_handle, &cbs, NULL);
  i2s_channel_enable(pcm5102a_handle);
}



void Pcm5102aTxTask(void * parameters){
  i2s_channel_write(pcm5102a_handle, src_buf, BLOCK_SIZE, &bytes_written, 20 / portTICK_PERIOD_MS);
  while(1){
    vTaskSuspend(NULL);
    for(int i = 0; i < BLOCK_SIZE; ++i){
      src_buf[i] = next_sample_32bit(&osc);
      //ESP_LOGI(TAG, "%ld", src_buf[i]);
    }
    i2s_channel_write(pcm5102a_handle, src_buf, BLOCK_SIZE, &bytes_written, 100 / portTICK_PERIOD_MS);
    
  }
  i2s_channel_disable(pcm5102a_handle);
  i2s_del_channel(pcm5102a_handle);
  vTaskDelete(NULL);
}

void init_pcm5102a(void){  
  init_oscillator(&osc, &sin_wave, 0.5, 440);
  init_i2s();
  xPcm5102aTxHandle = xTaskCreateStatic(Pcm5102aTxTask, "xPcm5102aTx", PCM5102A_TX_STACK_SIZE, (void*)0, 1, xPcm5102aTxStack, &xPcm5102aTxTCB);
  ESP_LOGI(TAG, "pcm5102a initialized");
}
