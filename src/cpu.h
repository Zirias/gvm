#ifndef CPU_H
#define CPU_H

#include <stdint.h>

typedef enum CpuFlags
{
    CF_ZERO     = 1<<0,
    CF_NEGATIVE = 1<<1,
    CF_CARRY    = 1<<2
} CpuFlags;

typedef enum CpuReg
{
    CR_A,
    CR_X,
    CR_Y
} CpuReg;

typedef struct Cpu Cpu;
typedef struct Ram Ram;

Cpu *Cpu_create(Ram *ram, uint16_t pc);
int Cpu_step(Cpu *self, char *dis);
uint16_t Cpu_pc(const Cpu *self);
CpuFlags Cpu_flags(const Cpu *self);
uint8_t Cpu_reg(const Cpu *self, CpuReg r);
void Cpu_destroy(Cpu *self);

#endif
