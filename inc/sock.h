// socket I/O
#ifndef _SOCK_H
#define _SOCK_H
#include "file.h"

#define SOCK_BUFF_SIZE 256 // TODO research the optimum

// buffer struct for better socket I/O
struct SOCK
{
    fd desc; // descriptor this socket consumes from
    char buff[SOCK_BUFF_SIZE]; // data goes here
    size_t len; // number of bytes in buffer
    size_t used; // number of bytes consumed
};

typedef struct SOCK SOCK;

// construct a SOCK with the given file descriptor, containing no data yet
SOCK mksock(fd desc);

// make a call to this SOCK's socket and overwrite its buffer
// return number of bytes acquired (and thus new len)
// return -1 on error (this SOCK is unchanged)
ssize_t sockrecv(SOCK* this, int flags);

// get one char from a socket
// returns 0 if the socket is closed
// returns -1 on error
int sgetc(fd sock, int flags);

// simpler function to read at most len - 1 bytes into a buffer of len size until the illegal char appears
// returns the number of chars read, and terminates the string
// returns len on failure, like snprintf
size_t readuntilchar(fd sock, size_t len, char buf[len], char illegal);

// write into a socket with formatting
int sockprintf(fd sock, const char* fmt, ...);

// send n bytes of an opened, readable file to a socket
// return the number of bytes read, or -1 on an error
ssize_t transmitfile(fd sock, fd file, size_t n);
#endif