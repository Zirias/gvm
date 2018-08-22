#include <stdlib.h>
#include <stdint.h>

#include "cpu.h"
#include "ram.h"
#include "opcode.h"

struct Cpu
{
    Ram *ram;
    CpuFlags flags;
    uint16_t pc;
    uint16_t sp;
    uint8_t stack[256];
    uint8_t regs[3];
};

Cpu *Cpu_create(Ram *ram, uint16_t pc)
{
    if (pc >= Ram_size(ram)) return 0;
    Cpu *self = malloc(sizeof *self);
    self->ram = ram;
    self->pc = pc;
    self->flags = 0;
    self->sp = 0;
    return self;
}

#define SR(x) do { \
    if ((x) & 1) self->flags |= CF_CARRY; \
    else self->flags &= ~CF_CARRY; \
    (x) >>= 1; \
} while(0)

#define SL(x) do { \
    if ((x) & 0x80) self->flags |= CF_CARRY; \
    else self->flags &= ~CF_CARRY; \
    (x) <<= 1; \
} while(0)

#define RR(x) do { \
    uint8_t c = 0x80 * !!(self->flags & CF_CARRY); \
    if ((x) & 1) self->flags |= CF_CARRY; \
    else self->flags &= ~CF_CARRY; \
    (x) = (x) >> 1 | c; \
} while(0)

#define RL(x) do { \
    uint8_t c = !!(self->flags & CF_CARRY); \
    if ((x) & 0x80) self->flags |= CF_CARRY; \
    else self->flags &= ~CF_CARRY; \
    (x) = (x) << 1 | c; \
} while(0)

#define NZ(x) do { \
    if ((x)) self->flags &= ~CF_ZERO; \
    else self->flags |= CF_ZERO; \
    if ((x) & 0x80) self->flags |= CF_NEGATIVE; \
    else self->flags &= ~CF_NEGATIVE; \
} while(0)

int Cpu_step(Cpu *self)
{
    int rc = 0;
    uint8_t op = Ram_get(self->ram, self->pc++);
    if (self->pc >= Ram_size(self->ram)) rc = -1;

    if ((op & O_AM_IMPLICIT) == O_AM_IMPLICIT)
    {
        if ((op & O_AM_JUMP) == O_AM_JUMP)
        {
            if (rc) return rc;
            uint16_t target;
            uint8_t arg1 = Ram_get(self->ram, self->pc++);
            if (op & O_AM_ABSOLUTE)
            {
                if (self->pc >= Ram_size(self->ram)) return -1;
                uint8_t arg2 = Ram_get(self->ram, self->pc++);
                target = arg2 << 8 | arg1;
            }
            else
            {
                int8_t diff = (int8_t) arg1;
                if (diff > self->pc) return -1;
                target = self->pc + diff;
            }
            int dojump = 1;
            switch (op & 0xfe)
            {
                case O_BSR:
                    if (self->sp == 256) return -1;
                    self->stack[self->sp++] = self->pc & 0xff;
                    if (self->sp == 256) return -1;
                    self->stack[self->sp++] = self->pc >> 8;
                    break;
                case O_BRA:
                    break;
                case O_BNE:
                    if (self->flags & CF_ZERO) dojump = 0;
                    break;
                case O_BEQ:
                    if (!(self->flags & CF_ZERO)) dojump = 0;
                    break;
                case O_BPL:
                    if (self->flags & CF_NEGATIVE) dojump = 0;
                    break;
                case O_BMI:
                    if (!(self->flags & CF_NEGATIVE)) dojump = 0;
                    break;
                case O_BCC:
                    if (self->flags & CF_CARRY) dojump = 0;
                    break;
                case O_BCS:
                    if (!(self->flags & CF_CARRY)) dojump = 0;
                    break;
                default:
                    return -1;
            }
            if (dojump)
            {
                rc = 0;
                if (target >= Ram_size(self->ram)) return -1;
                self->pc = target;
            }
        }
        else
        {
            switch (op)
            {
                case O_RTS:
                    if (self->sp == 0) return -1;
                    self->pc = self->stack[--self->sp] << 8;
                    if (self->sp == 0) return -1;
                    self->pc |= self->stack[--self->sp];
                    if (self->pc >= Ram_size(self->ram)) return -1;
                    break;
                case O_SRA:
                    SR(self->regs[CR_A]);
                    NZ(self->regs[CR_A]);
                    break;
                case O_SLA:
                    SL(self->regs[CR_A]);
                    NZ(self->regs[CR_A]);
                    break;
                case O_RRA:
                    RR(self->regs[CR_A]);
                    NZ(self->regs[CR_A]);
                    break;
                case O_RLA:
                    RL(self->regs[CR_A]);
                    NZ(self->regs[CR_A]);
                    break;
                case O_INA:
                    ++self->regs[CR_A];
                    NZ(self->regs[CR_A]);
                    break;
                case O_DEA:
                    --self->regs[CR_A];
                    NZ(self->regs[CR_A]);
                    break;
                case O_INX:
                    ++self->regs[CR_X];
                    NZ(self->regs[CR_X]);
                    break;
                case O_DEX:
                    --self->regs[CR_X];
                    NZ(self->regs[CR_X]);
                    break;
                case O_INY:
                    ++self->regs[CR_Y];
                    NZ(self->regs[CR_Y]);
                    break;
                case O_DEY:
                    --self->regs[CR_Y];
                    NZ(self->regs[CR_Y]);
                    break;
                case O_SEZ:
                    self->flags |= CF_ZERO;
                    break;
                case O_CLZ:
                    self->flags &= ~CF_ZERO;
                    break;
                case O_SEN:
                    self->flags |= CF_NEGATIVE;
                    break;
                case O_CLN:
                    self->flags &= ~CF_NEGATIVE;
                    break;
                case O_SEC:
                    self->flags |= CF_CARRY;
                    break;
                case O_CLC:
                    self->flags &= ~CF_CARRY;
                    break;
                case O_TAX:
                    self->regs[CR_X] = self->regs[CR_A];
                    NZ(self->regs[CR_X]);
                    break;
                case O_TXA:
                    self->regs[CR_A] = self->regs[CR_X];
                    NZ(self->regs[CR_A]);
                    break;
                case O_TAY:
                    self->regs[CR_Y] = self->regs[CR_A];
                    NZ(self->regs[CR_Y]);
                    break;
                case O_TYA:
                    self->regs[CR_A] = self->regs[CR_Y];
                    NZ(self->regs[CR_A]);
                    break;
                case O_TXY:
                    self->regs[CR_Y] = self->regs[CR_X];
                    NZ(self->regs[CR_Y]);
                    break;
                case O_TYX:
                    self->regs[CR_X] = self->regs[CR_Y];
                    NZ(self->regs[CR_X]);
                    break;
                case O_HLT:
                default:
                    return -1;
            }
        }
    }
    else
    {
        uint8_t arg1, arg2, v;
        uint16_t addr, ind;
        if (rc) return rc;
        switch (op & 7)
        {
            case O_AM_IMMEDIATE:
                addr = self->pc++;
                if (self->pc >= Ram_size(self->ram)) rc = -1;
                break;
            case O_AM_ABSOLUTE:
                arg1 = Ram_get(self->ram, self->pc++);
                if (self->pc >= Ram_size(self->ram)) return -1;
                arg2 = Ram_get(self->ram, self->pc++);
                if (self->pc >= Ram_size(self->ram)) rc = -1;
                addr = arg2 << 8 | arg1;
                break;
            case O_AM_ZP_ABS:
                arg1 = Ram_get(self->ram, self->pc++);
                if (self->pc >= Ram_size(self->ram)) rc = -1;
                addr = arg1;
                break;
            case O_AM_IDX_X:
                arg1 = Ram_get(self->ram, self->pc++);
                if (self->pc >= Ram_size(self->ram)) return -1;
                arg2 = Ram_get(self->ram, self->pc++);
                if (self->pc >= Ram_size(self->ram)) rc = -1;
                addr = (arg2 << 8 | arg1) + self->regs[CR_X];
                break;
            case O_AM_ZP_IDX_X:
                arg1 = Ram_get(self->ram, self->pc++);
                if (self->pc >= Ram_size(self->ram)) rc = -1;
                addr = arg1 + self->regs[CR_X];
                break;
            case O_AM_IDX_Y:
                arg1 = Ram_get(self->ram, self->pc++);
                if (self->pc >= Ram_size(self->ram)) return -1;
                arg2 = Ram_get(self->ram, self->pc++);
                if (self->pc >= Ram_size(self->ram)) rc = -1;
                addr = (arg2 << 8 | arg1) + self->regs[CR_Y];
                break;
            case O_AM_ZP_IDX_Y:
                arg1 = Ram_get(self->ram, self->pc++);
                if (self->pc >= Ram_size(self->ram)) rc = -1;
                addr = arg1 + self->regs[CR_Y];
                break;
            case O_AM_ZP_IND_Y:
                arg1 = Ram_get(self->ram, self->pc++);
                if (self->pc >= Ram_size(self->ram)) rc = -1;
                if ((size_t)arg1 + 1 > Ram_size(self->ram)) return -1;
                ind = Ram_get(self->ram, arg1) |
                    Ram_get(self->ram, arg1 + 1) << 8;
                addr = ind + self->regs[CR_Y];
                break;
        }
        if (addr > Ram_size(self->ram)) return -1;
        v = Ram_get(self->ram, addr);
        switch (op & 0xf8)
        {
            case O_LDA:
                self->regs[CR_A] = v;
                NZ(self->regs[CR_A]);
                break;
            case O_STA:
                Ram_set(self->ram, addr, self->regs[CR_A]);
                break;
            case O_LDX:
                self->regs[CR_X] = v;
                NZ(self->regs[CR_X]);
                break;
            case O_STX:
                Ram_set(self->ram, addr, self->regs[CR_X]);
                break;
            case O_LDY:
                self->regs[CR_Y] = v;
                NZ(self->regs[CR_Y]);
                break;
            case O_STY:
                Ram_set(self->ram, addr, self->regs[CR_Y]);
                break;
            case O_AND:
                self->regs[CR_A] &= v;
                NZ(self->regs[CR_A]);
                break;
            case O_ORA:
                self->regs[CR_A] |= v;
                NZ(self->regs[CR_A]);
                break;
            case O_EOR:
                self->regs[CR_A] ^= v;
                NZ(self->regs[CR_A]);
                break;
            case O_LSR:
                SR(v);
                NZ(v);
                Ram_set(self->ram, addr, v);
                break;
            case O_ASL:
                SL(v);
                NZ(v);
                Ram_set(self->ram, addr, v);
                break;
            case O_ROR:
                RR(v);
                NZ(v);
                Ram_set(self->ram, addr, v);
                break;
            case O_ROL:
                RL(v);
                NZ(v);
                Ram_set(self->ram, addr, v);
                break;
            case O_ADC:
                self->regs[CR_A] += v;
                if (self->flags & CF_CARRY) ++self->regs[CR_A];
                if (self->regs[CR_A] > v) self->flags &= ~CF_CARRY;
                else self->flags |= CF_CARRY;
                NZ(self->regs[CR_A]);
                break;
            case O_SBC:
                if (!(self->flags & CF_CARRY)) --self->regs[CR_A];
                self->regs[CR_A] -= v;
                if (self->regs[CR_A] < v) self->flags |= CF_CARRY;
                else self->flags &= ~CF_CARRY;
                NZ(self->regs[CR_A]);
                break;
            case O_INC:
                ++v;
                NZ(v);
                Ram_set(self->ram, addr, v);
                break;
            case O_DEC:
                --v;
                NZ(v);
                Ram_set(self->ram, addr, v);
                break;
            case O_CMP:
                v = self->regs[CR_A] - v;
                if (v < self->regs[CR_A]) self->flags |= CF_CARRY;
                else self->flags &= ~CF_CARRY;
                NZ(v);
                break;
            case O_CPX:
                v = self->regs[CR_X] - v;
                if (v < self->regs[CR_X]) self->flags |= CF_CARRY;
                else self->flags &= ~CF_CARRY;
                NZ(v);
                break;
            case O_CPY:
                v = self->regs[CR_Y] - v;
                if (v < self->regs[CR_Y]) self->flags |= CF_CARRY;
                else self->flags &= ~CF_CARRY;
                NZ(v);
                break;
            default:
                return -1;
        }
    }
    return rc;
}

uint16_t Cpu_pc(const Cpu *self)
{
    return self->pc;
}

CpuFlags Cpu_flags(const Cpu *self)
{
    return self->flags;
}

uint8_t Cpu_reg(const Cpu *self, CpuReg r)
{
    return self->regs[r];
}

void Cpu_destroy(Cpu *self)
{
    free(self);
}

