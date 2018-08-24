#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#ifdef BUILTIN_GETOPT
#include "builtin_getopt.h"
#else
#include <unistd.h>
#endif

#include "ram.h"
#include "cpu.h"

typedef enum mode
{
    M_XCODE,
    M_XRAM
} mode;

typedef enum dump
{
    D_NONE,
    D_BIN,
    D_HEX
} dump;

static Ram *createXcode(int hex, uint16_t load)
{
    Ram *ram = Ram_create(0x10000, 0);
    if (!ram) return 0;

    uint8_t *buf = malloc(0x10000 - load);
    if (!buf)
    {
        Ram_destroy(ram);
        return 0;
    }

    uint16_t sz = 0;

    if (hex)
    {
        unsigned val;
        int rc;

        while ((rc = scanf("%x", &val)) > 0)
        {
            if (val > 0xff)
            {
                fputs("parse error!\n", stderr);
                Ram_destroy(ram);
                free(buf);
                return 0;
            }
            buf[sz++] = val;
        }
        if (!rc)
        {
            fputs("parse error!\n", stderr);
            Ram_destroy(ram);
            free(buf);
            return 0;
        }
    }
    else
    {
        size_t szr;
        while ((szr = fread(buf + sz, 1, 0x10000 - load - sz, stdin)))
        {
            sz += szr;
        }
        if (!feof(stdin))
        {
            fputs("unknown loading error\n", stderr);
            Ram_destroy(ram);
            free(buf);
            return 0;
        }
    }

    if (Ram_load(ram, load, buf, sz) < 0)
    {
        fputs("unknown loading error\n", stderr);
        Ram_destroy(ram);
        free(buf);
        return 0;
    }

    free(buf);
    return ram;
}

static Ram *createXram(int hex)
{
    Ram *ram = Ram_create(0,0);
    if (!ram) return 0;

    if (hex)
    {
        unsigned val;
        int rc;
        while ((rc = scanf("%x", &val)) > 0)
        {
            if (val > 0xff)
            {
                fputs("parse error!\n", stderr);
                Ram_destroy(ram);
                return 0;
            }
            if (Ram_appendByte(ram, val) < 0)
            {
                fputs("loading error (input too large?)\n", stderr);
                Ram_destroy(ram);
                return 0;
            }
        }
        if (!rc)
        {
            fputs("parse error!\n", stderr);
            Ram_destroy(ram);
            return 0;
        }
    }
    else
    {
        uint8_t buf[0x1000];
        size_t sz;
        while ((sz = fread(buf, 1, 0x1000, stdin)))
        {
            if (Ram_append(ram, buf, sz) < 0)
            {
                fputs("loading error (input too large?)\n", stderr);
                Ram_destroy(ram);
                return 0;
            }
        }
        if (!feof(stdin))
        {
            fputs("unknown loading error\n", stderr);
            Ram_destroy(ram);
            return 0;
        }
    }

    return ram;
}

int main(int argc, char **argv)
{
    mode m = M_XCODE;
    dump d = D_NONE;
    uint16_t start = 0x100;
    int userstart = 0;
    int trace = 0;
    int hex = 0;
    char *convert = 0;
    int opt;

    if (!argv[0]) argv[0] = "gvm";
    while ((opt = getopt(argc, argv, "rs:htc:dx")) != -1)
    {
        switch (opt)
        {
            case 'r':
                m = M_XRAM;
                if (!userstart) start = 0;
                break;
            case 's':
                userstart = 1;
                start = atoi(optarg);
                break;
            case 't':
                trace = 1;
                break;
            case 'h':
                hex = 1;
                break;
            case 'c':
                convert = optarg;
                break;
            case 'd':
                d = D_BIN;
                break;
            case 'x':
                d = D_HEX;
                break;
            default:
                goto usage;
        }
    }
    if (optind < argc) goto usage;

    Ram *ram = 0;
    if (m == M_XRAM)
    {
        ram = createXram(hex);
    }
    else
    {
        ram = createXcode(hex, start);
    }
    if (!ram) return EXIT_FAILURE;

    Cpu *cpu = Cpu_create(ram, start);
    if (!cpu)
    {
        Ram_destroy(ram);
        return EXIT_FAILURE;
    }

    int rc = 0;
    while (rc >= 0)
    {
        if (trace)
        {
            CpuFlags f = Cpu_flags(cpu);
            fprintf(stderr, "PC:%04x - A:%02x X:%02x Y:%02x - [ %c %c %c ]\n",
                    Cpu_pc(cpu), Cpu_reg(cpu, CR_A),
                    Cpu_reg(cpu, CR_X), Cpu_reg(cpu, CR_Y),
                    f & CF_ZERO ? 'Z' : '_',
                    f & CF_NEGATIVE ? 'N' : '_',
                    f & CF_CARRY ? 'C' : '_');
            char dis[32];
            rc = Cpu_step(cpu, dis);
            fprintf(stderr, "%s\n", dis);
            fflush(stderr);
        }
        else rc = Cpu_step(cpu, 0);
    }
    if (trace)
    {
        fputs("=== terminated ===\n", stderr);
        fflush(stderr);
    }

    if (d)
    {
        int x = 0;
        for (size_t i = 0; i < Ram_size(ram); ++i)
        {
            if (d == D_HEX)
            {
                printf("%02x ", Ram_get(ram, i));
                if (++x == 26)
                {
                    x = 0;
                    puts("");
                }
            }
            else
            {
                putchar(Ram_get(ram, i));
            }
        }
        fflush(stdout);
    }

    Cpu_destroy(cpu);
    Ram_destroy(ram);
    return EXIT_SUCCESS;

usage:
    fprintf(stderr, "Usage: %s [-r] [-s startpc] [-h] [-t] [-c convfile] "
            "[-d] [-x]\n", argv[0]);
    return EXIT_FAILURE;
}
