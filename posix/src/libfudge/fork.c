#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>

#undef errno
extern int errno;

int fork()
{

    errno = ENOSYS;

    return -1;

}

