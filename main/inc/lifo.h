#include <stdio.h>
#include <stdint.h>
#include "esp_log.h"

#define LIFO_SIZE 5

typedef struct {
  uint8_t buf[LIFO_SIZE];
  uint8_t curr_pos;             // ensure initial value is 0
}Lifo;


uint8_t lifo_pop(Lifo *lifo);
void lifo_push(Lifo *lifo, uint8_t val);
void lifo_remove(Lifo *lifo, uint8_t val);
void lifo_print(Lifo *lifo);
void lifo_init(Lifo *lifo);
