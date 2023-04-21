#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "esp_log.h"

#include "midi.h"

#pragma once

#define ENV_ATTACK_TCO 0.9
#define ENV_DECAY_TCO exp(-11.05)
#define ENV_RELEASE_TCO exp(-11.05)

#define ENV_ATTACK_SAMPLES 2000
#define ENV_DECAY_SAMPLES 400000
#define ENV_SUSTAIN_LEVEL 0.5
#define ENV_RELEASE_SAMPLES 40000

#define ENV_ATTACK_SCALING 100
#define ENV_DECAY_SCALING 1000
#define ENV_SUSTAIN_SCALING 127
#define ENV_RELEASE_SCALING 1000

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


void update_envelope(EnvelopeGenerator * env);

void init_envelope(EnvelopeGenerator *env, EnvelopeSettings settings);

double process_curve(CurveGenerator *curve);

double process_envelope(EnvelopeGenerator *env, MidiMessageID event);

