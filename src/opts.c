#include "main.h"
#include "opts.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <unistd.h>
#include <getopt.h>

const char* usagemsg = "Usage: cserve [-h] [-p <port number>] <root directory>";

// list of all options recognised by cserve
// verbose, version, quiet, max connections, etc.
const struct cserveopt optlist[] = {
    {'h', "Display this help message and exit", NULL, false},
    {'p', "Specify the port to serve on", "80", true},
    // {'d', "Specify directory to serve", NULL},
};
const size_t optc = size(optlist);

int parseopts(struct cserveconf* opts, int argc, char* argv[])
{
    size_t optstrsz = 0;
    for (size_t i = 0; i < optc; i++)
    {
        optstrsz++;
        if (optlist[i].takesarg)
            optstrsz++;
    }
    char valid[optstrsz + 1];
    for (size_t i = 0; i < optc; i++)
    {
        valid[i] = optlist[i].symb;
        if (optlist[i].takesarg)
            valid[++i] = ':';
    }
    valid[optstrsz] = '\0';

    int c;
    while ((c = getopt(argc, argv, valid)) != -1)
    {
        switch (c)
        {
            case 'h':
                opts->help = true;
                break;
            case 'p':
            {
                for (size_t i = 0; optarg[i]; i++)
                {
                    if (!isdigit((unsigned char) optarg[i]))
                    {
                        fputs("Error: given port '", stderr); // 19 chars
                        i += 19;                              // we're breaking this loop anyway
                        fprintf(stderr, "%s' invalid\n", optarg);
                        for (size_t j = 0; j < i; j++)
                            putc(' ', stderr);
                        fputs("^\n", stderr);
                        return 1;
                    }
                }
                opts->portn = atoi(optarg);
                break;
            }
        }
    }
    return optind;
}

void printopts(FILE* file)
{
    for (size_t i = 0; i < optc; i++)
    {
        for (int j = 0; j < 2; j++)
            putc(' ', file);
        putc('-', file);
        putc(optlist[i].symb, file);
        for (int j = 0; j < 4; j++)
            putc(' ', file);
        fputs(optlist[i].help, file);
        putc('\n', file);
        if (optlist[i].deflt)
        {
            for (int j = 0; j < 8; j++)
                putc(' ', file);
            fprintf(file, "Default: %s\n", optlist[i].deflt);
        }
    }
}