#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "ram.h"
#include "cpu.h"

int main(int argc, char **argv)
{
    uint16_t start = 0;
    int rc = 0;
    unsigned val;
    
    if (argc > 1) start = atoi(argv[1]);
    Ram *ram = Ram_create(0,0);
    if (!ram) return EXIT_FAILURE;

    while ((rc = scanf("%x", &val)) > 0)
    {
        if (val > 0xff)
        {
            fputs("parse error!\n", stderr);
            Ram_destroy(ram);
            return EXIT_FAILURE;
        }
        if (Ram_appendByte(ram, val) < 0)
        {
            fputs("loading error (input too large?)\n", stderr);
            Ram_destroy(ram);
            return EXIT_FAILURE;
        }
    }
    if (!rc)
    {
        fputs("parse error!\n", stderr);
        Ram_destroy(ram);
        return EXIT_FAILURE;
    }

    Cpu *cpu = Cpu_create(ram, start);
    if (!cpu)
    {
        Ram_destroy(ram);
        return EXIT_FAILURE;
    }

    rc = 0;
    while (rc >= 0)
    {
        CpuFlags f = Cpu_flags(cpu);
        fprintf(stderr, "PC:%04x - A:%02x X:%02x - [ %c %c %c ]\n",
                Cpu_pc(cpu), Cpu_reg(cpu, CR_A),
                Cpu_reg(cpu, CR_X),
                f & CF_ZERO ? 'Z' : '_',
                f & CF_NEGATIVE ? 'N' : '_',
                f & CF_CARRY ? 'C' : '_');
        char dis[32];
        rc = Cpu_step(cpu, dis);
        fprintf(stderr, "%s\n", dis);
        fflush(stderr);
    }
    fputs("=== terminated ===\n", stderr);
    fflush(stderr);

    int x = 0;
    for (size_t i = 0; i < Ram_size(ram); ++i)
    {
        printf("%02x ", Ram_get(ram, i));
        if (++x == 26)
        {
            x = 0;
            puts("");
        }
    }

    Cpu_destroy(cpu);
    Ram_destroy(ram);
    return EXIT_SUCCESS;
}
