#include <_ansi.h>
#include <_syslist.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#undef errno
extern int errno;

int fstat(int fd, struct stat *st)
{

    errno = ENOSYS;

    return -1;

}

