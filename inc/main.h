#ifndef _MAIN_H
#define _MAIN_H

#define size(arr) (sizeof(arr) / sizeof(arr[0]))

// print a formatted string to stderr, along with the current ERRNO message
void eprintf(const char* format, ...);
#endif