#include "env.h"

static const char *TAG = "env";

void update_envelope(EnvelopeGenerator * env){
  // init attack curve values
  env->attack.coefficient = exp((-log((1 + env->settings.attack_tco) / env->settings.attack_tco) / env->settings.attack_samples));
  env->attack.offset = (1 + env->settings.attack_tco) * (1 - env->attack.coefficient);

  // init decay curve values
  env->decay.coefficient = exp((-log((1 + env->settings.decay_tco) / env->settings.decay_tco) / env->settings.decay_samples));
  env->decay.offset = (env->settings.sustain_level - env->settings.decay_tco) * (1 - env->decay.coefficient);

  // init release curve values
  env->release.coefficient = exp((-log((1 + env->settings.release_tco) / env->settings.release_tco) / env->settings.release_samples));
  env->release.offset = (0 - env->settings.release_tco) * (1 - env->release.coefficient);
}

void init_envelope(EnvelopeGenerator *env, EnvelopeSettings settings){
  env->settings = settings;
  update_envelope(env);
  env->attack.y_prev = 0;
  env->decay.y_prev = 1;
  env->decay.y_prev = 1;
  env->amplitude_prev = 0;
  env->state = OFF;
}

double process_curve(CurveGenerator *curve){
  double ret = curve->y_prev;
  curve->y_prev = curve->offset + curve->coefficient * curve->y_prev;
  return ret;
}

double process_envelope(EnvelopeGenerator *env, MidiMessageID event){
  double ret = env->amplitude_prev;
  switch(event){
    case NOTE_ON:
      env->state = ATTACK;
      env->attack.y_prev = env->amplitude_prev;
      env->amplitude_prev = process_curve(&(env->attack));
      if(env->amplitude_prev > 1.0){
        env->amplitude_prev = 1.0;
        env->state = DECAY;
      }
      break;
    case NOTE_OFF:
      env->state = RELEASE;
      env->release.y_prev = env->amplitude_prev;
      env->amplitude_prev = process_curve(&(env->release));
      if(env->amplitude_prev < 0){
        env->amplitude_prev = 0;
      }
      break;
    default:
      switch(env->state){
        case ATTACK:
          env->amplitude_prev = process_curve(&(env->attack));
          if(env->amplitude_prev > 1.0){
            env->amplitude_prev = 1.0;
            env->state = DECAY;
            env->decay.y_prev = 1.0;
          }
          break;
        case DECAY:
          env->amplitude_prev = process_curve(&(env->decay));
          if(env->amplitude_prev < env->settings.sustain_level){
            env->amplitude_prev = env->settings.sustain_level;
            env->state = SUSTAIN;
          }
          break;
        case RELEASE:
          env->amplitude_prev = process_curve(&(env->release));
          if(env->amplitude_prev < 0){
            env->amplitude_prev = 0;
            env->state = OFF;
          }
          break;
        case OFF:
          env->amplitude_prev = 0;
        default:
          break;

      }
      break;

  }
  return ret;
}