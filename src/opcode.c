#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include "opcode.h"

static const char *multimode[] =
{
    "LDA",
    "STA",
    "LDX",
    "STX",
    "LDY",
    "STY",
    "AND",
    "ORA",
    "EOR",
    "LSR",
    "ASL",
    "ROR",
    "ROL",
    "ADC",
    "SBC",
    "INC",
    "DEC",
    "CMP",
    "CPX",
    "CPY",
    "WUD",
    "WSD",
    "WCH",
    "WTX"
};

static const char *implicit[] =
{
    "HLT",
    "RTS",
    "SRA",
    "SLA",
    "RRA",
    "RLA",
    "INA",
    "DEA",
    "INX",
    "DEX",
    "INY",
    "DEY",
    "SEZ",
    "CLZ",
    "SEN",
    "CLN",
    "SEC",
    "CLC",
    "TAX",
    "TXA",
    "TAY",
    "TYA",
    "TXY",
    "TYX",
    "PHA",
    "PLA",
    "PHX",
    "PLX",
    "WNL",
    "WTB",
    "WSP",
    "RUD",
    "RSD",
    "RCH",
    "RTX"
};

static const char *branch[] =
{
    "BSR",
    "BRA",
    "BNE",
    "BEQ",
    "BPL",
    "BMI",
    "BCC",
    "BCS"
};

static void uc(char *upper, size_t n, const char *str)
{
    if (!n) return;
    const unsigned char *c = (const unsigned char *)str;
    for (size_t i = 0; *c && i < n-1; ++i, ++c)
    {
        *upper++ = toupper(*c);
    }
    *upper = 0;
}

int Opcode_fromString(Opcode *oc, const char *str, Opcode am)
{
    if (strlen(str) != 3) return ILL_INST;

    char opstr[4];
    uc(opstr, 4, str);

    int found = 0;
    uint8_t i;
    if (*opstr == 'B')
    {
	for (i = 0; i < sizeof branch / sizeof *branch; ++i)
	{
	    if (!strcmp(opstr, branch[i]))
	    {
		found = 1;
		break;
	    }
	}
	if (!found) return ILL_INST;
	if (am < O_AM_RELATIVE || am > O_AM_ABSOLUTE) return ILL_AM;
	*oc = O_AM_JUMP | i<<1 | am;
	return 0;
    }
    for (i = 0; i < sizeof multimode / sizeof *multimode; ++i)
    {
	if (!strcmp(opstr, multimode[i]))
	{
	    found = 1;
	    break;
	}
    }
    if (found)
    {
	if (am < O_AM_IMMEDIATE || am > O_AM_ZP_IND_Y) return ILL_AM;
	*oc = i<<3 | am;
	return 0;
    }
    for (i = 0; i < sizeof implicit / sizeof *implicit; ++i)
    {
	if (!strcmp(opstr, implicit[i]))
	{
	    found = 1;
	    break;
	}
    }
    if (!found) return ILL_INST;
    if (am != O_AM_IMPLICIT) return ILL_AM;
    *oc = i | am;
    return 0;
}

