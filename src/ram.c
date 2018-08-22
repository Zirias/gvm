#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "ram.h"

#define RAM_PAGESIZE 0x1000

struct Ram
{
    size_t size;
    size_t capa;
    uint8_t *m;
};

Ram *Ram_create(size_t size, const uint8_t *content)
{
    if (size > RAM_MAXSIZE) return 0;
    size_t capa = size + size % RAM_PAGESIZE;
    Ram *self = malloc(sizeof *self);
    if (!self) return 0;
    if (size)
    {
        self->m = malloc(capa);
        if (!self->m)
        {
            free(self);
            return 0;
        }
    }
    else
    {
        self->m = 0;
    }
    self->size = size;
    self->capa = capa;
    if (content)
    {
        memcpy(self->m, content, size);
    }
    return self;
}

int Ram_load(Ram *self, uint16_t at, const uint8_t *data, size_t size)
{
    if (size > self->size || size + at > self->size) return -1;
    memcpy(self->m + at, data, size);
    return 0;
}

int Ram_appendByte(Ram *self, uint8_t byte)
{
    if (self->size == RAM_MAXSIZE) return -1;
    if (self->size == self->capa)
    {
        size_t nc = self->capa + RAM_PAGESIZE;
        uint8_t *nm = realloc(self->m, nc);
        if (!nm) return -1;
        self->m = nm;
        self->capa = nc;
    }
    self->m[self->size++] = byte;
    return 0;
}

int Ram_append(Ram *self, const uint8_t *data, size_t size)
{
    if (self->size + size > RAM_MAXSIZE) return -1;
    if (self->size + size > self->capa)
    {
        size_t nc = self->size + size + ((self->size + size) % RAM_PAGESIZE);
        uint8_t *nm = realloc(self->m, nc);
        if (!nm) return -1;
        self->m = nm;
        self->capa = nc;
    }
    memcpy(self->m + self->size, data, size);
    self->size += size;
    return 0;
}

int Ram_set(Ram *self, uint16_t at, uint8_t byte)
{
    if (at >= self->size) return -1;
    self->m[at] = byte;
}

uint8_t Ram_get(const Ram *self, uint16_t at)
{
    if (self->size && (at >= self->size)) return 0;
    return self->m[at];
}

size_t Ram_size(const Ram *self)
{
    return self->size;
}

const uint8_t *Ram_contents(const Ram *self)
{
    return self->m;
}

void Ram_destroy(Ram *self)
{
    if (!self) return;
    free(self->m);
    free(self);
}

