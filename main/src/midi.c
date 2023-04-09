#include "midi.h"
#include "midi_pitch_map.h"
#include "pcm5102a.h"

MidiController mc;

SemaphoreHandle_t xMidiUpdateHandle;
static StaticSemaphore_t xMidiUpdateSCB;

static SemaphoreHandle_t xMidiControllerHandle;
static StaticSemaphore_t xMidiControllerSCB;

static QueueHandle_t xMidiUartHandle;
static QueueHandle_t xMidiMsgHandle;
static StaticQueue_t xMidiMsgQCB;
uint8_t xMidiMsgQueueBuf[ MIDI_MSG_QUEUE_LEN * sizeof(MidiMessage) ];

static TaskHandle_t xMidiRecvHandle;
static StackType_t xMidiRecvStack[MIDI_RECV_STACK_SIZE];
static StaticTask_t xMidiRecvTCB;

static TaskHandle_t xMidiProcHandle;
static StackType_t xMidiProcStack[MIDI_RECV_STACK_SIZE];
static StaticTask_t xMidiProcTCB;

#define PITCHBEND_RANGE 1
#define PITCHBEND_RATIO_MAX 1.12244
#define PITCHBEND_RATIO_MIN 0.89081
#define PITCHBEND_VALUE_MAX 0x7F7F
#define PITCHBEND_VALUE_MID 0x4000
#define PITCHBEND_VALUE_MIN 0x0000

static const char *TAG = "midi";

double pitchbend_to_pitch(double pitch, uint16_t pitch_bend){
  if(pitch_bend < PITCHBEND_VALUE_MID){
    double x = (double) pitch_bend / (PITCHBEND_VALUE_MID - PITCHBEND_VALUE_MIN);
    double m = 1 - PITCHBEND_RATIO_MIN;
    double b = (double) PITCHBEND_VALUE_MIN / (PITCHBEND_VALUE_MID - PITCHBEND_VALUE_MIN);
    b *= m;
    b = PITCHBEND_RATIO_MIN - b;
    return m * x + b;
  }
  double x = (double) pitch_bend / (PITCHBEND_VALUE_MAX - PITCHBEND_VALUE_MID);
  double m = PITCHBEND_RATIO_MAX - 1;
  double b = (double) PITCHBEND_VALUE_MID / (PITCHBEND_VALUE_MAX - PITCHBEND_VALUE_MID);
  b *= m;
  b = 1 - b;
  return m * x + b;
}

void MidiRecvTask(void *parameters){
  uart_config_t uart_config = {
    .baud_rate = MIDI_UART_BAUD_RATE,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_DEFAULT,
  };
  ESP_ERROR_CHECK(uart_driver_install(MIDI_UART_PORT_NUM, MIDI_BUF_SIZE * 2, MIDI_BUF_SIZE * 2, 20, &xMidiUartHandle, 0));
  ESP_ERROR_CHECK(uart_param_config(MIDI_UART_PORT_NUM, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(MIDI_UART_PORT_NUM, 21, MIDI_UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
  
  while(1){
    uart_event_t event;
    if(xQueueReceive(xMidiUartHandle, (void * )&event, (TickType_t)portMAX_DELAY)) {
      if(event.type == UART_DATA){
        MidiMessage msg;
        size_t length;
        ESP_ERROR_CHECK(uart_get_buffered_data_len(MIDI_UART_PORT_NUM, &length));
        for(int i = 0; i < length; i += 3){
          int size = uart_read_bytes(MIDI_UART_PORT_NUM, &msg, 3, 20 / portTICK_PERIOD_MS);
          if(size){
            xQueueSend(xMidiMsgHandle, &msg, 20 / portTICK_PERIOD_MS);
          }
        }
      }
    }
  }
  uart_driver_delete(MIDI_UART_PORT_NUM);
  vTaskDelete(NULL);
}

void MidiProcTask(void *parameters) {
  mc.gate = 0;
  mc.pitch_bend = 1;
  while(1){
    MidiMessage msg;
    if(xQueueReceive(xMidiMsgHandle, (void *)&msg, (TickType_t)portMAX_DELAY)){
      
      uint8_t msg_id = msg.id >> 4;
      xSemaphoreTake(xMidiControllerHandle, 1000 / portTICK_PERIOD_MS);
      switch(msg_id){
        case NOTE_OFF:
          if(mc.gate > 0){
            mc.gate--;
          }
          ESP_LOGD(TAG, "NOTE OFF: mgs.f1=%d, pitch=%f, velocity=%d", msg.f1, MidiPitchMap[msg.f1], msg.f2);
          xTaskNotify(xPcm5102aTxHandle, 1U << NOTE_OFF, eSetBits);
          break;
        case NOTE_ON:
          mc.gate++;
          mc.pitch = MidiPitchMap[msg.f1];
          ESP_LOGD(TAG, "NOTE ON: mgs.f1=%d, pitch=%f, velocity=%d", msg.f1, MidiPitchMap[msg.f1], msg.f2);
          xTaskNotify(xPcm5102aTxHandle, 1U << NOTE_ON, eSetBits);
          break;
        case MOD_WHEEL:
          mc.mod_wheel = msg.f2;
          ESP_LOGD(TAG, "MOD WHEEL: control_num=%d, control_val=%d", msg.f1, msg.f2);
          break;
        case PITCH_BEND:
          uint16_t pb = (msg.f2 << 8) + (msg.f1);
          mc.pitch_bend = pitchbend_to_pitch(mc.pitch, pb);
          ESP_LOGD(TAG, "PITCH BEND: 0x%04X, pitch_bend=%f", pb, mc.pitch_bend);
          xTaskNotify(xPcm5102aTxHandle, 1U << PITCH_BEND, eSetBits);
          break;
        default:
          ESP_LOGD(TAG, "msg_id=%X", msg_id);
          break;
      }
      xSemaphoreGive(xMidiControllerHandle);
      
    }
  }
}

void init_midi(void){
  xMidiControllerHandle = xSemaphoreCreateMutexStatic(&xMidiControllerSCB);
  xMidiUpdateHandle = xSemaphoreCreateBinaryStatic(&xMidiUpdateSCB);
  xMidiMsgHandle = xQueueCreateStatic(MIDI_MSG_QUEUE_LEN, sizeof(MidiMessage), xMidiMsgQueueBuf, &xMidiMsgQCB);
  xMidiProcHandle = xTaskCreateStatic(MidiProcTask, "xMidiProc", MIDI_RECV_STACK_SIZE, (void*)0, 1, xMidiProcStack, &xMidiProcTCB);
  xMidiRecvHandle = xTaskCreateStatic(MidiRecvTask, "xMidiRecv", MIDI_RECV_STACK_SIZE, (void*)0, 5, xMidiRecvStack, &xMidiRecvTCB);
  ESP_LOGI(TAG, "midi initialized");
}