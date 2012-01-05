#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>

#undef errno
extern int errno;

int lseek(int file, int ptr, int dir)
{

    errno = ENOSYS;

    return -1;

}

