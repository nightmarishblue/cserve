// socket I/O
#ifndef _SOCK_H
#define _SOCK_H
#include "file.h"

#define SBUFF_SIZE 256 // TODO research the optimum

// buffer struct for better socket I/O
typedef struct
{
    fd desc; // descriptor this socket consumes from
    char buff[SBUFF_SIZE]; // data goes here
    size_t len; // number of bytes in buffer
    size_t used; // number of bytes consumed
} SBUFF;

// construct a SBUFF with the given file descriptor, containing no data yet
SBUFF mksbuff(fd desc);

// make a read from this SBUFF's socket and overwrite its buffer
// return number of bytes acquired (and thus new len)
// return -1 on error (this SBUFF is unchanged)
ssize_t sbuffrecv(SBUFF* this, int flags);

// consume and return the next character in this SBUFF's buffer
// refills with sbuffrecv if needed
// return -1 if none is available
int sbuffgetc(SBUFF* this);

// get one char from a socket
// returns 0 if the socket is closed
// returns -1 on error
int sgetc(fd sock, int flags); // TODO prune

// peek at the next character in this SBUFF's buffer, refilling if needed
// return -1 if none available
int sbuffpeek(SBUFF* this);

// simpler function to read at most len - 1 bytes into a buffer of len size until the illegal char appears
// returns the number of chars read, and terminates the string
// returns len on failure, like snprintf
size_t readuntilchar(SBUFF* sock, size_t len, char buf[len], char illegal);

// write into a socket with formatting
int sockprintf(fd sock, const char* fmt, ...);

// send n bytes of an opened, readable file to a socket
// return the number of bytes read, or -1 on an error
ssize_t transmitfile(fd sock, fd file, size_t n);
#endif