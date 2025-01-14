// socket I/O
#ifndef _SOCK_H
#define _SOCK_H
#include "file.h"

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