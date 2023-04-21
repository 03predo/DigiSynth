#include <stdio.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/uart.h"
#include "driver/gpio.h"

#pragma once

#define MIDI_RECV_STACK_SIZE 4096
#define MIDI_UART_PORT_NUM 2
#define MIDI_UART_BAUD_RATE 31250
#define MIDI_UART_RX_PIN 23
#define MIDI_BUF_SIZE 256

#define MIDI_MSG_QUEUE_LEN 20

#define MIDI_MOD_WHEEL_CTRL_NUM 1

#define MIDI_FADER_BANK_BASE 12
#define MIDI_FADER_BANK_SIZE 8

#define MIDI_KNOB_BANK_BASE 22
#define MIDI_KNOB_BANK_SIZE 8

#define MIDI_BUTTON_BANK_BASE 32
#define MIDI_BUTTON_BANK_SIZE 8

#define MIDI_ATTACK_CTRL_FADER 0
#define MIDI_DECAY_CTRL_FADER 1
#define MIDI_SUSTAIN_CTRL_FADER 2
#define MIDI_RELEASE_CTRL_FADER 3

#define MIDI_SIN_BUTTON 0
#define MIDI_SAWTOOTH_BUTTON 1
#define MIDI_SQUARE_BUTTON 2

typedef enum {
  NOTE_OFF = 0x8,
  NOTE_ON = 0x9,
  CTRL = 0xB,
  PITCH_BEND = 0xE, // 0x4000 is middle, 0x7F7F is top, 0x0000 is bottom
}MidiMessageID;

typedef struct MidiMessage {
  uint8_t id;
  uint8_t f1;
  uint8_t f2;
} MidiMessage;

typedef struct MidiController {
  uint8_t gate;
  double pitch;
  uint8_t mod_wheel;
  uint8_t fader_bank[MIDI_FADER_BANK_SIZE];
  uint8_t knob_bank[MIDI_KNOB_BANK_SIZE];
  uint8_t button_bank;
  double pitch_bend;
} MidiController;

extern MidiController mc;
extern SemaphoreHandle_t xMidiUpdateHandle;

void init_midi(void);