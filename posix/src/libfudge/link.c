#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>

#undef errno
extern int errno;

int link(char *existing, char *new)
{

    errno = ENOSYS;

    return -1;

}

