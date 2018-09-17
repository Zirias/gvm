#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "help.h"
#include "vm.h"
#include "asm.h"

int main(int argc, char **argv)
{
    if (!argv[0]) argv[0] = "gvm";

    if (argc == 2 && (!strcmp(argv[1], "-?") || !strcmp(argv[1], "-h")
		|| !strcmp(argv[1], "--help")))
    {
	showhelp(argv[0]);
	return EXIT_SUCCESS;
    }

    if (argc > 1 && !strcmp(argv[1], "as"))
    {
        return asmain(--argc, ++argv);
    }

    if (strlen(argv[0]) > 4)
    {
        char *cmdname = strrchr(argv[0], '/');
        if (!cmdname) cmdname = strrchr(argv[0], '\\');
        if (!cmdname)
        {
            cmdname = argv[0];
        }
        else ++cmdname;
        char *ext = strchr(cmdname, '.');
        if (ext) *ext = 0;
        for (unsigned char *c = (unsigned char *)cmdname; *c; ++c)
        {
            *c = tolower(*c);
        }
        if (!strcmp(cmdname, "gvmas"))
        {
            return asmain(argc, argv);
        }
    }

    return vmmain(argc, argv);
}

