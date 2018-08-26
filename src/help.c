#include <stdio.h>

#include "help.h"

void showusage(const char *prg)
{
    fprintf(stderr, "Usage: %s [-r] [-s startpc] [-h] [-t] [-c convfile] "
            "[-d] [-x] <program>\n"
	    "       %s asm <source>\n"
	    "       %s -?|-h|--help\n"
	    , prg, prg, prg);
}

void showhelp(const char *prg)
{
    fprintf(stderr, "GVM 0.0a1 - an 8bit virtual machine\n"
	    "Felix Palmen <felix@palmen-it.de>\n\n"
	    " %s [-r] [-s startpc] [-h] [-t] [-c convfile] "
	    "[-d] [-x] <program>\n"
	    "    Run a program in the virtual machine.\n\n"
	    "    -r: input is the whole RAM (default: input is a program to "
	    "load into\n"
	    "        64KB RAM at $100 / 256)\n"
	    "    -s startpc: start execution at <startpc> (default: 256 in "
	    "normal mode,\n"
	    "                0 in -r mode)\n"
	    "    -h: input is a hex file (default: binary)\n"
	    "    -t: enable tracing of execution to stderr\n"
	    "    -c convfile: translate program to a different set of opcodes "
	    "given in\n"
	    "                 <convfile> during execution\n"
	    "    -d: dump final contents of RAM in binary\n"
	    "    -x: dump final contents of RAM in hex\n"
	    "    <program>: the program to load or the RAM to use in -r mode\n"
	    "\n"
	    " %s asm <source>\n"
	    "    Assemble <source> to binary bytecode\n\n"
	    " %s -?|-h|--help\n"
	    "    Show this help message\n"
	    , prg, prg, prg);
}
