#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "converter.h"
#include "ram.h"

struct Converter
{
    Ram *input;
    Ram *output;
    uint8_t map[256];
};

static void idmap(Converter *self)
{
    for (uint16_t x = 0; x < 256; ++x)
    {
        self->map[x] = x;
    }
}

Converter *Converter_create(const Ram *ram)
{
    if (!ram) return 0;
    Converter *self = malloc(sizeof *self);
    if (!self) return 0;
    self->input = Ram_clone(ram);
    self->output = Ram_clone(ram);
    if (!self->input || !self->output)
    {
        Converter_destroy(self);
        return 0;
    }
    idmap(self);
    return self;
}

int Converter_readTable(Converter *self, FILE *convtable)
{
    idmap(self);
    unsigned from, to;
    int rc;
    while ((rc = fscanf(convtable, "%x", &from)) > 0)
    {
        if ((rc = fscanf(convtable, "%x", &to)) < 1)
        {
            idmap(self);
            return -1;
        }
        if (from > 0xff || to > 0xff)
        {
            idmap(self);
            return -1;
        }
        self->map[from] = to;
    }
    if (!rc)
    {
        idmap(self);
        return -1;
    }
    return 0;
}

int Converter_writeOpcode(const Converter *self, uint8_t opcode, uint16_t at)
{
    if (at >= Ram_size(self->input)) return -1;
    Ram_set(self->input, at, self->map[opcode]);
    Ram_set(self->output, at, self->map[opcode]);
    return 0;
}

int Converter_writeData(const Converter *self, uint8_t data, uint16_t at)
{
    if (at >= Ram_size(self->input)) return -1;
    Ram_set(self->output, at, data);
    return 0;
}

const Ram *Converter_input(const Converter *self)
{
    return self->input;
}

const Ram *Converter_output(const Converter *self)
{
    return self->output;
}

void Converter_destroy(Converter *self)
{
    if (!self) return;
    Ram_destroy(self->input);
    Ram_destroy(self->output);
    free(self);
}

