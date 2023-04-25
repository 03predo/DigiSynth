#include "lifo.h"

static const char *TAG = "lifo";

void lifo_init(Lifo *lifo){
  lifo->curr_pos = 0;
  for(int i = 0; i < LIFO_SIZE; ++i){
    lifo->buf[i] = 0;
  }
}

void lifo_push(Lifo *lifo, uint8_t val){
  lifo->curr_pos = (lifo->curr_pos + 1) % LIFO_SIZE;
  lifo->buf[lifo->curr_pos] = val;
  return;
}

uint8_t lifo_pop(Lifo *lifo){
  lifo->buf[lifo->curr_pos] = 0;
  lifo->curr_pos = (LIFO_SIZE + lifo->curr_pos - 1) % LIFO_SIZE;
  return 0;
}

void lifo_remove(Lifo *lifo, uint8_t val){
  for(int i = 0; i < LIFO_SIZE; ++i){
    if(lifo->buf[i] == val){
      for(int j = i; j < i + LIFO_SIZE; ++j){
        if((j % LIFO_SIZE) == lifo->curr_pos){
          lifo->buf[lifo->curr_pos] = 0;
          lifo->curr_pos = (LIFO_SIZE + lifo->curr_pos - 1) % LIFO_SIZE;
          break;
        }
        lifo->buf[j % LIFO_SIZE] = lifo->buf[((j + 1) % LIFO_SIZE)];

      }
      break;
    }
  }
  return;
}

void lifo_print(Lifo *lifo){
  ESP_LOGI(TAG, "lifo->curr_pos=%d", lifo->curr_pos);
  for(int i = 0; i < LIFO_SIZE; ++i){
    ESP_LOGI(TAG, "lifo[%d]: %d", i, lifo->buf[i]);
  }
}