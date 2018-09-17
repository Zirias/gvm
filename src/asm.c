#define _POSIX_C_SOURCE 200112L

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#ifdef BUILTIN_GETOPT
#include "builtin_getopt.h"
#else
#include <unistd.h>
#endif

#include "help.h"

int asmain(int argc, char **argv)
{
    return EXIT_FAILURE;
}

