#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>

#undef errno
extern int errno;

int close(int fd)
{

    return call_close(fd) - 1;

}

