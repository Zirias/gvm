#ifndef OPCODE_H
#define OPCODE_H

typedef enum Opcode
{
    // Addressing modes
    O_AM_IMPLICIT   = 0xc0,
    O_AM_JUMP       = 0xf0,
    O_AM_IMMEDIATE  = 0,
    O_AM_RELATIVE   = 0,
    O_AM_ZP_ABS     = 2,
    O_AM_ZP_IDX_X   = 4,

    O_LDA           = 0 << 3,
    O_STA           = 1 << 3,
    O_LDX           = 2 << 3,
    O_STX           = 3 << 3,
    O_AND           = 6 << 3,
    O_ORA           = 7 << 3,
    O_EOR           = 8 << 3,
    O_LSR           = 9 << 3,
    O_ASL           = 10 << 3,
    O_ROR           = 11 << 3,
    O_ROL           = 12 << 3,
    O_ADC           = 13 << 3,
    O_INC           = 15 << 3,
    O_DEC           = 16 << 3,
    O_CMP           = 17 << 3,
    O_CPX           = 18 << 3,

    O_HLT           = 0xc0,
    O_INX           = 0xc8,
    O_DEX           = 0xc9,
    O_SEC           = 0xd0,
    O_CLC           = 0xd1,

    O_BRA           = 0x79 << 1,
    O_BNE           = 0x7a << 1,
    O_BEQ           = 0x7b << 1,
    O_BPL           = 0x7c << 1,
    O_BMI           = 0x7d << 1,
    O_BCC           = 0x7e << 1,
    O_BCS           = 0x7f << 1

} Opcode;

#endif
