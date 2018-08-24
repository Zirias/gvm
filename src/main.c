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
#include "converter.h"

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

static void dumpRamHex(const Ram *ram)
{
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
    puts("");
    fflush(stdout);
}

static void dumpRam(const Ram *ram)
{
    for (size_t i = 0; i < Ram_size(ram); ++i)
    {
        putchar(Ram_get(ram, i));
    }
    fflush(stdout);
}

static Ram *createXcode(FILE *prg, int hex, uint16_t load)
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

        while ((rc = fscanf(prg, "%x", &val)) > 0)
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
        while ((szr = fread(buf + sz, 1, 0x10000 - load - sz, prg)))
        {
            sz += szr;
        }
        if (!feof(prg))
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

static Ram *createXram(FILE *prg, int hex)
{
    Ram *ram = Ram_create(0,0);
    if (!ram) return 0;

    if (hex)
    {
        unsigned val;
        int rc;
        while ((rc = fscanf(prg, "%x", &val)) > 0)
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
        while ((sz = fread(buf, 1, 0x1000, prg)))
        {
            if (Ram_append(ram, buf, sz) < 0)
            {
                fputs("loading error (input too large?)\n", stderr);
                Ram_destroy(ram);
                return 0;
            }
        }
        if (!feof(prg))
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
    FILE *convtable = 0;
    int opt;

    Ram *ram = 0;
    Converter *converter = 0;
    Cpu *cpu = 0;

    setvbuf(stdin, 0, _IONBF, 0);

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
                if (convtable) goto usage;
                convtable = fopen(optarg, "r");
                if (!convtable)
                {
                    fprintf(stderr, "Error opening %s for reading.\n", optarg);
                    return EXIT_FAILURE;
                }
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
    if (optind == argc || optind < argc-1) goto usage;

    FILE *prg = fopen(argv[optind], "r");
    if (!prg)
    {
        fprintf(stderr, "Error opening %s for reading.\n", argv[optind]);
        goto error;
    }

    if (m == M_XRAM)
    {
        ram = createXram(prg, hex);
    }
    else
    {
        ram = createXcode(prg, hex, start);
    }
    fclose(prg);
    if (!ram) goto error;

    if (convtable)
    {
        converter = Converter_create(ram);
        if (!converter) goto error;
        if (Converter_readTable(converter, convtable) < 0)
        {
            fputs("Error reading conversion table.\n", stderr);
            goto error;
        }
        fclose(convtable);
        convtable = 0;
    }

    cpu = Cpu_create(ram, start, converter);
    if (!cpu) goto error;

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
        if (d == D_HEX) dumpRamHex(ram);
        else dumpRam(ram);
    }

    if (converter)
    {
        puts("Converted input:");
        dumpRamHex(Converter_input(converter));
        puts("Converted output:");
        dumpRamHex(Converter_output(converter));
    }

    if (convtable) fclose(convtable);
    Converter_destroy(converter);
    Cpu_destroy(cpu);
    Ram_destroy(ram);
    return EXIT_SUCCESS;

error:
    if (convtable) fclose(convtable);
    Converter_destroy(converter);
    Cpu_destroy(cpu);
    Ram_destroy(ram);
    return EXIT_FAILURE;

usage:
    if (convtable) fclose(convtable);
    fprintf(stderr, "Usage: %s [-r] [-s startpc] [-h] [-t] [-c convfile] "
            "[-d] [-x] <program>\n", argv[0]);
    return EXIT_FAILURE;
}
