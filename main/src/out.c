#include "out.h"
#include "midi.h"
#include "env.h"

#define BLOCK_SIZE 512
#define STACK_SIZE 8192

Oscillator osc;
EnvelopeGenerator amp_env;

i2s_chan_handle_t pcm5102a_handle;

TaskHandle_t xOutputHandle;
static StackType_t xOutputStack[STACK_SIZE];
static StaticTask_t xOutputTCB;

static int32_t tx_buf[BLOCK_SIZE] = { 0 };

static const char *TAG = "out";

void init_i2s(void){
  i2s_chan_config_t chan_cfg = {
    .id = I2S_NUM_AUTO,
    .role = I2S_ROLE_MASTER,
    .dma_desc_num = 6,      // number of buffers being used
    .dma_frame_num = 256,  // dma_frame_num <= ((4092 * 8) / (slot_num * data_bit_width)), rounded down to avoid byte loss
    .auto_clear = true,
  };

  i2s_new_channel(&chan_cfg, &pcm5102a_handle, NULL);

  i2s_std_config_t std_cfg = {
    .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
    .slot_cfg = {
      .data_bit_width = BIT_DEPTH,
      .slot_bit_width = BIT_DEPTH * 2,
      .slot_mode = I2S_SLOT_MODE_MONO,
      .slot_mask = I2S_STD_SLOT_LEFT,
      .ws_width = BIT_DEPTH,
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
    .on_sent = NULL,
    .on_send_q_ovf = NULL,
  };
  i2s_channel_register_event_callback(pcm5102a_handle, &cbs, NULL);
  i2s_channel_enable(pcm5102a_handle);
}

uint8_t process_notification(uint32_t notification, uint8_t gate){
  static uint8_t button_bank = 0;
  if(notification & (1U << CTRL)){
    amp_env.settings.attack_samples = mc.fader_bank[MIDI_ATTACK_CTRL_FADER] * ENV_ATTACK_SCALING;
    amp_env.settings.decay_samples = mc.fader_bank[MIDI_DECAY_CTRL_FADER] * ENV_DECAY_SCALING;
    amp_env.settings.sustain_level = ((double) mc.fader_bank[MIDI_SUSTAIN_CTRL_FADER]) / ENV_SUSTAIN_SCALING;
    amp_env.settings.release_samples = mc.fader_bank[MIDI_RELEASE_CTRL_FADER] * ENV_RELEASE_SCALING;
    update_envelope(&amp_env);

    uint8_t mask = ~(button_bank) & (mc.button_bank);
    button_bank = mc.button_bank;
    if(mask & (1 << MIDI_SIN_BUTTON)){
      ESP_LOGD(TAG, "changing to sin wave");
      osc.wave_function = &sin_wave;
      osc.angle = 0.0;
    }else if(mask & (1 << MIDI_SAWTOOTH_BUTTON)){
      ESP_LOGD(TAG, "changing to sawtooth wave");
      osc.wave_function = &sawtooth_wave;
      osc.angle = 0.0;
    }else if(mask & (1 << MIDI_SQUARE_BUTTON)){
      ESP_LOGD(TAG, "changing to square wave");
      osc.wave_function = &square_wave;
      osc.angle = 0.0;
    }
  }
  if(notification & (1U << NOTE_ON)){
    osc.amplitude = process_envelope(&amp_env, NOTE_ON);
  }
  if(notification & ((1U << NOTE_OFF) | (1U << NOTE_ON) | (1U << PITCH_BEND))){
    osc.offset = (mc.pitch * mc.pitch_bend) / (SAMPLE_RATE * 2);
    return mc.gate;
  }
  return gate;
}

void OutputTask(void *parameters){
  size_t bytes_written = 0;
  uint8_t gate = 0;
  while(1){
    uint32_t notification;
    if(xTaskNotifyWait(0, 0xFFFFFFFF, &notification, 1) == pdTRUE){
      gate = process_notification(notification, gate);
    }
    if(gate == 0){
      osc.amplitude = process_envelope(&amp_env, NOTE_OFF);
    }
    for(int i = 0; i < BLOCK_SIZE; ++i){
      tx_buf[i] = next_sample(&osc);
      osc.amplitude = process_envelope(&amp_env, 0);
    }
    
    i2s_channel_write(pcm5102a_handle, tx_buf, sizeof(tx_buf), &bytes_written, 1000 / portTICK_PERIOD_MS);
    if(bytes_written != sizeof(tx_buf)){
      ESP_LOGW(TAG, "bytes_written=%d", bytes_written);
    }
  }
  i2s_channel_disable(pcm5102a_handle);
  i2s_del_channel(pcm5102a_handle);
  vTaskDelete(NULL);
}

void init_output(void){
  init_i2s();
  
  EnvelopeSettings settings = {
    .attack_samples = ENV_ATTACK_SAMPLES,
    .attack_tco = ENV_ATTACK_TCO,
    .decay_samples = ENV_DECAY_SAMPLES,
    .decay_tco = ENV_DECAY_TCO,
    .sustain_level = ENV_SUSTAIN_LEVEL,
    .release_samples = ENV_RELEASE_SAMPLES,
    .release_tco = ENV_RELEASE_TCO
  };
  init_envelope(&amp_env, settings);
  init_oscillator(&osc, &sin_wave, 1, 440);
  
  xOutputHandle = xTaskCreateStatic(OutputTask, "xPcm5102aTx", STACK_SIZE, (void*)0, 3, xOutputStack, &xOutputTCB);
  ESP_LOGI(TAG, "output initialized");
}
