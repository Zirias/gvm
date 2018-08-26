#include <stdlib.h>
#include <string.h>

#include "help.h"
#include "vm.h"

int main(int argc, char **argv)
{
    if (!argv[0]) argv[0] = "gvm";

    if (argc < 2)
    {
	showusage(argv[0]);
	return EXIT_FAILURE;
    }

    if (argc == 2 && (!strcmp(argv[1], "-?") || !strcmp(argv[1], "-h")
		|| !strcmp(argv[1], "--help")))
    {
	showhelp(argv[0]);
	return EXIT_SUCCESS;
    }

    return vmmain(argc, argv);
}
