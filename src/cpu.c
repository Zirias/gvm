#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "cpu.h"
#include "ram.h"
#include "converter.h"
#include "opcode.h"

struct Cpu
{
    Ram *ram;
    Converter *conv;
    CpuFlags flags;
    uint8_t pc;
    uint8_t regs[2];
};

Cpu *Cpu_create(Ram *ram, uint16_t pc, Converter *conv)
{
    if (pc >= Ram_size(ram)) return 0;
    Cpu *self = malloc(sizeof *self);
    self->ram = ram;
    self->conv = conv;
    self->pc = pc;
    self->flags = 0;
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

static void logByte(char *dis, int off, uint8_t byte)
{
    char buf[3];
    if (dis)
    {
        sprintf(buf, "%02x", byte);
        memcpy(dis+off, buf, 2);
    }
}

static void logInst(char *dis, const char *inst)
{
    if (dis)
    {
        memcpy(dis+12, inst, strlen(inst));
    }
}

static void logRel(char *dis, int8_t off)
{
    char buf[5];
    if (dis)
    {
        sprintf(buf, "%+d", off);
        memcpy(dis+16, buf, strlen(buf));
    }
}

static void logImm(char *dis, int8_t arg)
{
    char buf[5];
    if (dis)
    {
        sprintf(buf, "#$%02x", arg);
        memcpy(dis+16, buf, 4);
    }
}

static void logZp(char *dis, uint8_t addr, char *index)
{
    char buf[6];
    if (dis)
    {
        sprintf(buf, "$%02x", addr);
        if (index) strcat(buf, index);
        memcpy(dis+16, buf, strlen(buf));
    }
}

static void logRes(char *dis, uint8_t res)
{
    char buf[6];
    if (dis)
    {
        sprintf(buf, "; $%02x", res);
        memcpy(dis+26, buf, 5);
    }
}

int Cpu_step(Cpu *self, char *dis)
{
    if (dis) strcpy(dis, "                               ");
    int rc = 0;
    uint8_t op = Ram_get(self->ram, self->pc);
    if (self->conv) Converter_writeOpcode(self->conv, op, self->pc);
    if (++self->pc >= Ram_size(self->ram)) rc = -1;
    logByte(dis, 0, op);

    if ((op & O_AM_IMPLICIT) == O_AM_IMPLICIT)
    {
        if ((op & O_AM_JUMP) == O_AM_JUMP)
        {
            if (rc) return rc;
            uint16_t target;
            uint8_t arg1 = Ram_get(self->ram, self->pc++);
            logByte(dis, 3, arg1);
            if (op & 1)
            {
                logInst(dis, "ILL");
                return -1;
            }
            else
            {
                int8_t diff = (int8_t) arg1;
                logRel(dis, diff);
                if (self->pc + diff < 0) return -1;
                target = self->pc + diff;
            }
            int dojump = 1;
            switch (op & 0xfe)
            {
                case O_BRA:
                    logInst(dis, "BRA");
                    break;
                case O_BNE:
                    if (self->flags & CF_ZERO) dojump = 0;
                    logInst(dis, "BNE");
                    break;
                case O_BEQ:
                    if (!(self->flags & CF_ZERO)) dojump = 0;
                    logInst(dis, "BEQ");
                    break;
                case O_BPL:
                    if (self->flags & CF_NEGATIVE) dojump = 0;
                    logInst(dis, "BPL");
                    break;
                case O_BMI:
                    if (!(self->flags & CF_NEGATIVE)) dojump = 0;
                    logInst(dis, "BMI");
                    break;
                case O_BCC:
                    if (self->flags & CF_CARRY) dojump = 0;
                    logInst(dis, "BCC");
                    break;
                case O_BCS:
                    if (!(self->flags & CF_CARRY)) dojump = 0;
                    logInst(dis, "BCS");
                    break;
                default:
                    logInst(dis, "ILL");
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
                case O_INX:
                    logInst(dis, "INX");
                    ++self->regs[CR_X];
                    NZ(self->regs[CR_X]);
                    break;
                case O_DEX:
                    logInst(dis, "DEX");
                    --self->regs[CR_X];
                    NZ(self->regs[CR_X]);
                    break;
                case O_SEC:
                    logInst(dis, "SEC");
                    self->flags |= CF_CARRY;
                    break;
                case O_CLC:
                    logInst(dis, "CLC");
                    self->flags &= ~CF_CARRY;
                    break;
                case O_HLT:
                    logInst(dis, "HLT");
                    return -1;
                default:
                    logInst(dis, "ILL");
                    return -1;
            }
        }
    }
    else
    {
        uint8_t arg1, v;
        uint16_t addr;
        int carry;
        if (rc) return rc;
        switch (op & 7)
        {
            case O_AM_IMMEDIATE:
                arg1 = Ram_get(self->ram, self->pc);
                logByte(dis, 3, arg1);
                logImm(dis, arg1);
                addr = self->pc++;
                if (self->pc >= Ram_size(self->ram)) rc = -1;
                break;
            case O_AM_ZP_ABS:
                arg1 = Ram_get(self->ram, self->pc++);
                logByte(dis, 3, arg1);
                if (self->pc >= Ram_size(self->ram)) rc = -1;
                addr = arg1;
                logZp(dis, arg1, 0);
                break;
            case O_AM_ZP_IDX_X:
                arg1 = Ram_get(self->ram, self->pc++);
                logByte(dis, 3, arg1);
                if (self->pc >= Ram_size(self->ram)) rc = -1;
                addr = arg1 + self->regs[CR_X];
                logZp(dis, arg1, ",X");
                break;
            default:
                logInst(dis, "ILL");
                return -1;
        }
        if (addr > Ram_size(self->ram)) return -1;
        v = Ram_get(self->ram, addr);
        switch (op & 0xf8)
        {
            case O_LDA:
                logInst(dis, "LDA");
                self->regs[CR_A] = v;
                NZ(self->regs[CR_A]);
                break;
            case O_STA:
                logInst(dis, "STA");
                Ram_set(self->ram, addr, self->regs[CR_A]);
                if (self->conv) Converter_writeData(self->conv,
                        self->regs[CR_A], addr);
                break;
            case O_LDX:
                logInst(dis, "LDX");
                self->regs[CR_X] = v;
                NZ(self->regs[CR_X]);
                break;
            case O_STX:
                logInst(dis, "STX");
                Ram_set(self->ram, addr, self->regs[CR_X]);
                if (self->conv) Converter_writeData(self->conv,
                        self->regs[CR_X], addr);
                break;
            case O_AND:
                logInst(dis, "AND");
                self->regs[CR_A] &= v;
                NZ(self->regs[CR_A]);
                break;
            case O_ORA:
                logInst(dis, "ORA");
                self->regs[CR_A] |= v;
                NZ(self->regs[CR_A]);
                break;
            case O_EOR:
                logInst(dis, "EOR");
                self->regs[CR_A] ^= v;
                NZ(self->regs[CR_A]);
                break;
            case O_LSR:
                logInst(dis, "LSR");
                SR(v);
                NZ(v);
                Ram_set(self->ram, addr, v);
                if (self->conv) Converter_writeData(self->conv, v, addr);
                logRes(dis, v);
                break;
            case O_ASL:
                logInst(dis, "ASL");
                SL(v);
                NZ(v);
                Ram_set(self->ram, addr, v);
                if (self->conv) Converter_writeData(self->conv, v, addr);
                logRes(dis, v);
                break;
            case O_ROR:
                logInst(dis, "ROR");
                RR(v);
                NZ(v);
                Ram_set(self->ram, addr, v);
                if (self->conv) Converter_writeData(self->conv, v, addr);
                logRes(dis, v);
                break;
            case O_ROL:
                logInst(dis, "ROL");
                RL(v);
                NZ(v);
                Ram_set(self->ram, addr, v);
                if (self->conv) Converter_writeData(self->conv, v, addr);
                logRes(dis, v);
                break;
            case O_ADC:
                logInst(dis, "ADC");
                carry = self->flags & CF_CARRY;
                self->regs[CR_A] += v;
                if (self->regs[CR_A] >= v) self->flags &= ~CF_CARRY;
                else self->flags |= CF_CARRY;
                if (carry)
                {
                    if (!++self->regs[CR_A]) self->flags |= CF_CARRY;
                }
                NZ(self->regs[CR_A]);
                break;
            case O_INC:
                logInst(dis, "INC");
                ++v;
                NZ(v);
                Ram_set(self->ram, addr, v);
                if (self->conv) Converter_writeData(self->conv, v, addr);
                logRes(dis, v);
                break;
            case O_DEC:
                logInst(dis, "DEC");
                --v;
                NZ(v);
                Ram_set(self->ram, addr, v);
                if (self->conv) Converter_writeData(self->conv, v, addr);
                logRes(dis, v);
                break;
            case O_CMP:
                logInst(dis, "CMP");
                v = self->regs[CR_A] - v;
                if (v < self->regs[CR_A]) self->flags |= CF_CARRY;
                else self->flags &= ~CF_CARRY;
                NZ(v);
                break;
            case O_CPX:
                logInst(dis, "CPX");
                v = self->regs[CR_X] - v;
                if (v < self->regs[CR_X]) self->flags |= CF_CARRY;
                else self->flags &= ~CF_CARRY;
                NZ(v);
                break;
            default:
                logInst(dis, "ILL");
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

