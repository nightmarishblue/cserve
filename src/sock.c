#include "sock.h"
#include "main.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <sys/socket.h>

#include <sys/sendfile.h>

SOCK mksock(fd desc)
{
    SOCK out;
    out.desc = desc;
    out.len = 0;
    return out;
}

int sgetc(fd sock, int flags)
{
    unsigned char c[1];
    int i = recv(sock, c, 1, flags);
    if (i != 1) return i;
    return *c;
}

size_t readuntilchar(fd sock, const size_t len, char buf[len], char illegal)
{
    if (len == 0) return 0; // safer to simply exit if len is 0
    // go up len - 1 times - i sure hope GCC optimises this
    for (size_t i = 0; i < len - 1; i++)
    {
        int c = sgetc(sock, 0);
        if (c == illegal)
        {
            buf[i] = '\0';
            return i;
        }
        if (c <= 0) // error or socket close
        {
            buf[i] = '\0';
            return len;
        }
        buf[i] = c;
    }
    buf[len - 1] = '\0'; // terminate the string
    // peek at the next char and consume it if it's the breakchar
    if (sgetc(sock, MSG_PEEK) == illegal)
    {
        (void) sgetc(sock, 0);
        return len - 1; // also return the number of bytes consumed
    }
    return len; // return len on bad
}

int sockprintf(fd sock, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int i = vdprintf(sock, fmt, args); // possibly ifdef this
    va_end(args);
    return i;
}

ssize_t transmitfile(fd sock, fd file, size_t n)
{
    // TODO ensure this compiles on systems without sendfile
    ssize_t ret = sendfile(sock, file, NULL, n);
    if (ret == -1)
        eprintf("could not transmit file");
    return -1;
}