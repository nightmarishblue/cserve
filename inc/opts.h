#ifndef _OPTS_H
#define _OPTS_H
#include "file.h"

#include <stdbool.h>
#include <stdio.h>

// the program's current configuration after parsing options
struct cserveconf
{
    bool help;
    int portn;
    const char* srvdirpath;
    fd srvdir;
};

// the representation of a single option
struct cserveopt
{
    char symb;
    const char* help, *deflt;
    bool takesarg;
};

extern const char* usagemsg; // FIXME antipattern
extern const struct cserveconf* options;

// parse the input arguments and configure the struct as accordingly
int parseopts(struct cserveconf* opts, int argc, char* argv[]);
// print all registered options in a block to file (mostly for -h)
void printopts(FILE* file);
// parse the program's arguments and configure the global options object
void configure(int argc, char* argv[]);
#endif