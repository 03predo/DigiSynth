#include <stdint.h>
#include <string.h>

// raw values from adc
#define OFFSET 10
#define OCTAVES 3

typedef struct point {
  float x;
  float y;
} point;

const point C[] = {{.x=293, .y=65.41 }, {.x=717,  .y=130.81}, {.x=1151, .y=261.63}};
const point d[] = {{.x=327, .y=69.30 }, {.x=752,  .y=138.58}, {.x=1186, .y=277.18}};
const point D[] = {{.x=363, .y=73.42 }, {.x=785,  .y=146.83}, {.x=1222, .y=293.66}};
const point e[] = {{.x=399, .y=77.78 }, {.x=822,  .y=155.56}, {.x=1260, .y=311.13}};
const point E[] = {{.x=432, .y=82.41 }, {.x=857,  .y=164.81}, {.x=1296, .y=329.63}};
const point F[] = {{.x=466, .y=87.31 }, {.x=895,  .y=174.51}, {.x=1330, .y=349.23}};
const point g[] = {{.x=500, .y=92.50 }, {.x=930,  .y=185.00}, {.x=1367, .y=369.99}};
const point G[] = {{.x=542, .y=98.00 }, {.x=965,  .y=196.00}, {.x=1403, .y=392.00}};
const point a[] = {{.x=576, .y=103.83}, {.x=1001, .y=207.65}, {.x=1439, .y=415.30}};
const point A[] = {{.x=610, .y=110.00}, {.x=1041, .y=220.00}, {.x=1475, .y=440.00}};
const point b[] = {{.x=645, .y=116.54}, {.x=1079, .y=233.08}, {.x=1509, .y=466.16}};
const point B[] = {{.x=680, .y=123.47}, {.x=1114, .y=246.94}, {.x=1551, .y=493.88}};

const point* notes[] = {C, d, D, e, E, F, g, G, a, A, b, B};

static point note_list[12 * OCTAVES];

float adc_to_freq(int raw){
  static bool init = false;
  if(!(init)){
    size_t indx = 0;
    for(size_t i = 0; i < OCTAVES; ++i){
      for(size_t j = 0; j < 12; ++j){
        note_list[indx] = notes[j][i];
        indx++;
      }
    }
    init = true;
  }
  int start = 0, end = OCTAVES * 12 - 1;
  while(end - start != 1){
    size_t midpoint = (start + end)/2;
    if(raw > note_list[midpoint].x){
      start = midpoint;
    }else{
      end = midpoint;
    }
  }
  //ESP_LOGI("adc_to_freq", "raw: %d, start: (%f, %f), end: %f", raw, note_list[start].x, note_list[start].y, note_list[end].x);
  if(raw > (note_list[end].x - OFFSET)){
    //ESP_LOGI("adc_to_freq", "HERE1");
    return note_list[end].y;
  }else if(raw < (note_list[start].x + OFFSET)){
    //ESP_LOGI("adc_to_freq", "HERE2");
    return note_list[start].y;
  }else{
    //ESP_LOGI("adc_to_freq", "HERE3");
    float m = (note_list[end].y - note_list[start].y)/(note_list[end].x - note_list[start].x);
    float b = (note_list[end].x * note_list[start].y - note_list[start].x * note_list[end].y)/(note_list[end].x - note_list[start].x);
    return m * raw + b;
  }
}
