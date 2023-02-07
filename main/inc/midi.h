#include <stdio.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/uart.h"
#include "driver/gpio.h"

#define MIDI_RECV_STACK_SIZE 4096
#define MIDI_UART_PORT_NUM 2
#define MIDI_UART_BAUD_RATE 31250
#define MIDI_UART_RX_PIN 23
#define MIDI_BUF_SIZE 256

#define MIDI_MSG_QUEUE_LEN 20

enum MidiMessageID {
  NOTE_OFF = 0x8,
  NOTE_ON = 0x9,
  MOD_WHEEL = 0xB,
  PITCH_BEND = 0xE, // 0x4000 is middle, 0x7F7F is top, 0x0000 is bottom
};

typedef struct MidiMessage {
  uint8_t id;
  uint8_t f1;
  uint8_t f2;
} MidiMessage;

typedef struct MidiController {
  bool gate;
  double pitch;
  uint8_t mod_wheel;
  uint8_t pitch_bend;
} MidiController;

extern MidiController mc;

void init_midi(void);