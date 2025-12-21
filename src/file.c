#include "file.h"
#include "main.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h> // to check if openat2 is supported

#ifdef SYS_openat2
    #include <linux/openat2.h>
#endif

off_t filesize(fd file)
{
    struct stat finfo;
    if (fstat(file, &finfo) == -1)
    {
        eprintf("could not stat file");
        return -1;
    }
    return finfo.st_size;
}

bool isdir(fd file)
{
    struct stat dinfo;
    if (fstat(file, &dinfo) == -1)
        return false; // no way for the caller to tell if this is the reason for false...
    return S_ISDIR(dinfo.st_mode);
}

fd opend(const char* path)
{
    #ifdef O_DIRECTORY
        return open(path, O_RDONLY | O_DIRECTORY);
    #else
        fd dir = open(path, O_RDONLY);
        if (dir == -1) return -1;
        if (isdir(dir)) return dir;
        return -1;
    #endif
}

#ifdef SYS_openat2
    int openat2(fd dir, const char* path, const struct open_how* how, size_t size)
    {
        return syscall(SYS_openat2, dir, path, how, size);
    }
#endif

fd openunder(fd dir, const char* relpath, int flags)
{
    #ifdef SYS_openat2
        struct open_how how = { .flags = flags, .resolve = RESOLVE_IN_ROOT };
        return openat2(dir, relpath, &how, sizeof(how));
    #endif
    // TODO provide an alternative approach so this compiles outside Linux >= 5.6
}

bool canread(fd dir, const char* relpath)
{
    return faccessat(dir, relpath, R_OK, 0) == 0;
}

bool statfile(fd dir, const char* relpath, struct stat* result)
{
    // TODO alternative for older POSIX?
    return fstatat(dir, relpath, result, 0) == 0;
}