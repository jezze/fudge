#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>

#undef errno
extern int errno;

int kill(int pid, int sig)
{

    errno = ENOSYS;

    return -1;

}

