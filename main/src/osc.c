#include "osc.h"


double sin_wave(double x){
  return sin(2 * M_PI * x);
}

double sawtooth_wave(Oscillator* osc, double x){
  return 1 - 2*x;
}

double square_wave(double x){
  if(x < 0.5){
    return -1.0;
  }
  return 1.0;
}

double process(Oscillator * osc){
  double sample = osc->amplitude * (*osc->wave_function)(osc->angle);
  osc->angle += osc->offset;
  if(osc->angle > 1){
    osc->angle = 0;
  }
  return sample;
}

int16_t next_sample_16bit(Oscillator * osc){
  static int16_t max_amplitude = (1 << (BIT_DEPTH - 1)) - 1;
  return (int16_t) ((process(osc)) * max_amplitude);
}

int32_t next_sample_32bit(Oscillator * osc){
  static int32_t max_amplitude = (1 << (BIT_DEPTH - 1)) - 1;
  return (int32_t) ((process(osc)) * max_amplitude);
}

void init_oscillator(Oscillator * osc, double(*wave_function)(double), double amplitude, double frequency){
  osc->wave_function = wave_function;
  osc->amplitude = amplitude;
  osc->frequency = frequency;
  osc->angle = 0.0;
  osc->offset = frequency / SAMPLE_RATE;
}
