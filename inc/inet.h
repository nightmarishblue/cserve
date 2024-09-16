// networking and addressing
#ifndef _INET_H
#define _INET_H
#include "file.h"

#include <stdint.h>
#include <stdbool.h>

#include <netinet/in.h> // internet socket stuff

#define QUEUE_LEN 5 // how many connection requests to keep queued before we deal with them

// these functions print diagnostic error messages

// create an ipv4 endpoint as a stream and return its fd for reads and writes
fd crtsock();
// bind a socket to the given port, storing details in an external address struct
bool bindport(fd sock, struct sockaddr_in* addr, uint16_t port);
// put the given socket into the listening state
bool socklisten(fd sock);
// close a socket (only useful for the error output)
bool closesock(fd sock);
// shutdown and close a peer
bool closepeer(fd sock);
// accept a connection from an opened port and place its details into the sockaddr
// return the fd for the new socket
fd acceptconn(fd sock, struct sockaddr* clientaddr, socklen_t* addrlen);
// place the IP of an internat address into a string of up to len chars
// returning false on error
bool ipstr(const struct in_addr* iaddr, size_t len, char str[len]);
// with an internet address, place the string IP:PORT into str, up to the size limit
// return true if successful
bool addrstr(const struct sockaddr_in* iaddr, size_t size, char str[]);

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