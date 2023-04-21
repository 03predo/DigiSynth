#include "osc.h"


double sin_wave(double x){
  return sin(2 * M_PI * x);
}

double sawtooth_wave(double x){
  return (1 - 2*x);
}

double square_wave(double x){
  if(x < 0.5){
    return -1.0;
  }
  return 1.0;
}

double process(Oscillator * osc){
  double sample = osc->amplitude * osc->max_amplitude * (*osc->wave_function)(osc->angle);
  osc->angle += osc->offset;
  if(osc->angle > 1){
    osc->angle = 0;
  }
  return sample;
}

#if (BIT_DEPTH == 16)

int16_t next_sample(Oscillator * osc){
  static int16_t max_amplitude = (1 << (BIT_DEPTH - 1)) - 1;
  return (int16_t) ((process(osc)) * max_amplitude);
}

#elif (BIT_DEPTH == 32)

int32_t next_sample(Oscillator * osc){
  static int32_t max_amplitude = (1 << (BIT_DEPTH - 1)) - 1;
  return (int32_t) ((process(osc)) * max_amplitude);
}

#endif

void init_oscillator(Oscillator * osc, double(*wave_function)(double), double amplitude, double frequency){
  osc->wave_function = wave_function;
  osc->max_amplitude = amplitude;
  osc->frequency = frequency;
  osc->angle = 0.0;
  osc->amplitude = 1.0;
  // for some reason we are getting double the frequency on the output so this * 2 is a temp fix
  osc->offset = frequency / (SAMPLE_RATE * 2); 
}
