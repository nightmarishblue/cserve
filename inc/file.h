#ifndef _FILE_H
#define _FILE_H
#include <stdbool.h>
#include <sys/stat.h>

#include <sys/types.h>

typedef int fd;

// get the size of a file
// return -1 on failure
off_t filesize(fd file);

// check if the given file descriptor refers to a directory
bool isdir(fd file);

// open a directory
// return its file descriptor, or -1 on failure
// will fall back to using fstat if O_DIRECTORY is unavailable
fd opend(const char* path);

// open the given path relative to that of dir
// prevents escaping the given directory
// return the file descriptor, or -1 on error, in which case errno is set
fd openunder(fd dir, const char* relpath, int flags);

// check if we can read the given file
// errno is set if false
bool canread(fd dir, const char* relpath);

// stat the file relative to dir
// return success
bool statfile(fd dir, const char* relpath, struct stat* result);
#endif