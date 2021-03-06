#ifndef OPCODE_H
#define OPCODE_H

typedef enum Opcode
{
    // Addressing modes
    O_AM_IMPLICIT   = 0xc0,
    O_AM_JUMP       = 0xf0,
    O_AM_IMMEDIATE  = 0,
    O_AM_RELATIVE   = 0,
    O_AM_ABSOLUTE   = 1,
    O_AM_ZP_ABS     = 2,
    O_AM_IDX_X      = 3,
    O_AM_ZP_IDX_X   = 4,
    O_AM_IDX_Y      = 5,
    O_AM_ZP_IDX_Y   = 6,
    O_AM_ZP_IND_Y   = 7,

    O_LDA           = 0 << 3,
    O_STA           = 1 << 3,
    O_LDX           = 2 << 3,
    O_STX           = 3 << 3,
    O_LDY           = 4 << 3,
    O_STY           = 5 << 3,
    O_AND           = 6 << 3,
    O_ORA           = 7 << 3,
    O_EOR           = 8 << 3,
    O_LSR           = 9 << 3,
    O_ASL           = 10 << 3,
    O_ROR           = 11 << 3,
    O_ROL           = 12 << 3,
    O_ADC           = 13 << 3,
    O_SBC           = 14 << 3,
    O_INC           = 15 << 3,
    O_DEC           = 16 << 3,
    O_CMP           = 17 << 3,
    O_CPX           = 18 << 3,
    O_CPY           = 19 << 3,
    O_WUD           = 20 << 3,
    O_WSD           = 21 << 3,
    O_WCH           = 22 << 3,
    O_WTX           = 23 << 3,

    O_HLT           = 0xc0,
    O_RTS           = 0xc1,
    O_SRA           = 0xc2,
    O_SLA           = 0xc3,
    O_RRA           = 0xc4,
    O_RLA           = 0xc5,
    O_INA           = 0xc6,
    O_DEA           = 0xc7,
    O_INX           = 0xc8,
    O_DEX           = 0xc9,
    O_INY           = 0xca,
    O_DEY           = 0xcb,
    O_SEZ           = 0xcc,
    O_CLZ           = 0xcd,
    O_SEN           = 0xce,
    O_CLN           = 0xcf,
    O_SEC           = 0xd0,
    O_CLC           = 0xd1,
    O_TAX           = 0xd2,
    O_TXA           = 0xd3,
    O_TAY           = 0xd4,
    O_TYA           = 0xd5,
    O_TXY           = 0xd6,
    O_TYX           = 0xd7,
    O_PHA           = 0xd8,
    O_PLA           = 0xd9,
    O_PHX           = 0xda,
    O_PLX           = 0xdb,
    O_PHY           = 0xdc,
    O_PLY           = 0xdd,
    O_WNL           = 0xde,
    O_WTB           = 0xdf,
    O_WSP           = 0xe0,
    O_RUD           = 0xe1,
    O_RSD           = 0xe2,
    O_RCH           = 0xe3,
    O_RTX           = 0xe4,

    O_BSR           = 0x78 << 1,
    O_BRA           = 0x79 << 1,
    O_BNE           = 0x7a << 1,
    O_BEQ           = 0x7b << 1,
    O_BPL           = 0x7c << 1,
    O_BMI           = 0x7d << 1,
    O_BCC           = 0x7e << 1,
    O_BCS           = 0x7f << 1

} Opcode;

#define ILL_INST -1
#define ILL_AM -2

int Opcode_fromString(Opcode *oc, const char *str, Opcode am);

#endif
