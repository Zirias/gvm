#ifndef CONVERTER_H
#define CONVERTER_H

#include <stdio.h>
#include <stdint.h>

typedef struct Ram Ram;
typedef struct Converter Converter;

Converter *Converter_create(const Ram *ram);
int Converter_readTable(Converter *self, FILE *convtable);
int Converter_writeOpcode(const Converter *self, uint8_t opcode, uint16_t at);
int Converter_writeData(const Converter *self, uint8_t data, uint16_t at);
const Ram *Converter_input(const Converter *self);
const Ram *Converter_output(const Converter *self);
void Converter_destroy(Converter *self);

#endif
