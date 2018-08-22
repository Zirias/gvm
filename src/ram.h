#ifndef RAM_H
#define RAM_H

#include <stdint.h>

typedef struct Ram Ram;

Ram *Ram_create(uint16_t size);
void Ram_load(Ram *self, uint16_t at, uint8_t *data, uint16_t size);
void Ram_set(Ram *self, uint16_t at, uint8_t byte);
uint8_t Ram_get(const Ram *self, uint16_t at);
uint16_t Ram_size(const Ram *self);
const uint8_t *Ram_contents(const Ram *self);
void Ram_destroy(Ram *self);

#endif
