#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "ram.h"
#include "cpu.h"

static uint8_t input[0x10000];

int main(int argc, char **argv)
{
    uint16_t start = 0;
    uint16_t size = 0;
    if (argc > 1) start = atoi(argv[1]);
    int rc = 0;
    int ok = 0;
    unsigned val;
    while ((rc = scanf("%x", &val)) > 0)
    {
        if (val > 0xff)
        {
            fputs("parse error!\n", stderr);
            return EXIT_FAILURE;
        }
        if (ok && !size)
        {
            fputs("input too large!\n", stderr);
            return EXIT_FAILURE;
        }
        ok = 1;
        input[size++] =  val;
    }
    if (!rc)
    {
        fputs("parse error!\n", stderr);
        return EXIT_FAILURE;
    }

    Ram *ram = Ram_create(size);
    if (!ram) return EXIT_FAILURE;

    Ram_load(ram, 0, input, size);

    Cpu *cpu = Cpu_create(ram, start);
    if (!cpu)
    {
        Ram_destroy(ram);
        return EXIT_FAILURE;
    }

    do
    {
        CpuFlags f = Cpu_flags(cpu);
        fprintf(stderr, "PC:%04x - A:%02x X:%02x Y:%02x - [ %c %c %c ]\n",
                Cpu_pc(cpu), Cpu_reg(cpu, CR_A),
                Cpu_reg(cpu, CR_X), Cpu_reg(cpu, CR_Y),
                f & CF_ZERO ? 'Z' : '_',
                f & CF_NEGATIVE ? 'N' : '_',
                f & CF_CARRY ? 'C' : '_');
        fflush(stderr);
    } while (Cpu_step(cpu) >= 0);

    int x = 0;
    for (unsigned i = 0; i < (size ? size : 0x10000); ++i)
    {
        printf("%02x ", Ram_get(ram, i));
        if (++x == 26)
        {
            x = 0;
            puts("");
        }
    }

    return EXIT_SUCCESS;
}
