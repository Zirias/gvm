#ifndef BUILTIN_GETOPT_H
#define BUILTIN_GETOPT_H

extern int opterr;
extern int optind;
extern int optopt;
extern int optreset;
extern char *optarg;

int getopt(int nargc, char * const nargv[], const char *ostr);

#endif
