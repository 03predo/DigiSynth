#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "esp_log.h"

#pragma once

#define SAMPLE_RATE 16000
#define BIT_DEPTH 32
#define SAMPLE_TYPE(type) uint ## type ##  _t
#define MAX_AMPLITUDE (1 << (BIT_DEPTH - 1)) - 1;

typedef struct Oscillator {
  double (*wave_function)(double);  // return value is [0, 1] and period is 1
  double amplitude;                // value is [0, 1]
  double frequency;                // number of wave function cycles per second
  double offset;                   // step size per each sample, sample_rate / frequency
  double angle;                    // current angle in wave function
}Oscillator;

double sin_wave(double x);

double sawtooth_wave(double x);

double square_wave(double x);

#if (BIT_DEPTH == 16)

int16_t next_sample(Oscillator * osc);

#elif (BIT_DEPTH == 32)

int32_t next_sample(Oscillator * osc);

#else

#error "bit depth not supported"

#endif

void init_oscillator(Oscillator * osc, double(*wave_function)(double), double amplitude, double frequency);
