#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>

#undef errno
extern int errno;

int open(char *file, int flags, int mode)
{

    errno = ENOSYS;

    return -1;

}

