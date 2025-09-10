#include "opts.h"
#include "inet.h"
#include "http.h"
#include "sock.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <errno.h>

int main(int argc, char* argv[])
{
    configure(argc, argv); // will exit if unrecoverable invalid args are given

    struct sockaddr_in myaddr;
    fd sockid = crtsock();
    if (sockid == -1) return 1;
    if(!(bindport(sockid, &myaddr, options->portn) && socklisten(sockid)))
    {
        closesock(sockid);
        return 1;
    }

    while (true)
    {
        struct sockaddr_in clientaddr;
        socklen_t addrsz = sizeof(clientaddr);
        fd clisock = acceptconn(sockid, (struct sockaddr*) &clientaddr, &addrsz);
        if (clisock == -1) continue;

        char name[16];
        if (addrstr(&clientaddr, 16, name))
            printf("Received connection from %s\n", name);

        SBUFF socket = mksbuff(clisock);
        while (serve(&socket) && sbuffpeek(&socket) != -1);
        closepeer(clisock);
    }

    closesock(sockid);
    return 0;
}


void eprintf(const char* format, ...)
{
    // maybe quit if silent
    va_list args;
    va_start(args, format);
    fputs("Error: ", stderr); // would end up writing this 10 times otherwise
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, ": %s\n", strerror(errno));
}