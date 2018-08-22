#ifndef RAM_H
#define RAM_H

#include <stdint.h>

#define RAM_MAXSIZE 0x10000

typedef struct Ram Ram;

Ram *Ram_create(size_t size, const uint8_t *content);
int Ram_load(Ram *self, uint16_t at, const uint8_t *data, size_t size);
int Ram_appendByte(Ram *self, uint8_t byte);
int Ram_append(Ram *self, const uint8_t *data, size_t size);
int Ram_set(Ram *self, uint16_t at, uint8_t byte);
uint8_t Ram_get(const Ram *self, uint16_t at);
size_t Ram_size(const Ram *self);
const uint8_t *Ram_contents(const Ram *self);
void Ram_destroy(Ram *self);

#endif
