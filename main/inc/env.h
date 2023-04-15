#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "esp_log.h"

#include "midi.h"

#pragma once

typedef struct {
  double coefficient;
  double offset;
  double y_prev;
}CurveGenerator;

typedef struct {
  double attack_tco;
  double decay_tco;
  double release_tco;
  double sustain_level;
  uint32_t attack_samples;
  uint32_t decay_samples;
  uint32_t release_samples;
}EnvelopeSettings;

typedef enum {
  ATTACK = 0,
  DECAY,
  SUSTAIN,
  RELEASE,
  OFF,
}EnvelopeState;

typedef struct {
  EnvelopeSettings settings;
  CurveGenerator attack;
  CurveGenerator decay;
  CurveGenerator release;
  EnvelopeState state;
  double amplitude_prev;
}EnvelopeGenerator;



void init_envelope(EnvelopeGenerator *env, EnvelopeSettings settings);

double process_curve(CurveGenerator *curve);

double process_envelope(EnvelopeGenerator *env, MidiMessageID event);

