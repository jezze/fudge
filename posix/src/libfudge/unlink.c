#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>

#undef errno
extern int errno;

int unlink(char *name)
{

    errno = ENOSYS;

    return -1;

}

