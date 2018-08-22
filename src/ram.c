#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "ram.h"

struct Ram
{
    uint16_t size;
    uint8_t m[];
};

Ram *Ram_create(uint16_t size)
{
    size_t sz = size ? size : 0x10000;
    Ram *self = malloc(sizeof *self + sz);
    if (!self) return 0;
    self->size = size;
    return self;
}

void Ram_load(Ram *self, uint16_t at, uint8_t *data, uint16_t size)
{
    if (self->size && !size) return;
    if (self->size && (at >= self->size || at + size > self->size)) return;
    memcpy(self->m + at, data, size ? size : 0x10000);
}

void Ram_set(Ram *self, uint16_t at, uint8_t byte)
{
    if (self->size && (at >= self->size)) return;
    self->m[at] = byte;
}

uint8_t Ram_get(const Ram *self, uint16_t at)
{
    if (self->size && (at >= self->size)) return 0;
    return self->m[at];
}

uint16_t Ram_size(const Ram *self)
{
    return self->size;
}

const uint8_t *Ram_contents(const Ram *self)
{
    return self->m;
}

void Ram_destroy(Ram *self)
{
    free(self);
}

