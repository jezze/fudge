#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include <sys/types.h>

#undef errno
extern int errno;

int chown(const char *path, uid_t owner, gid_t group)
{

    errno = ENOSYS;

    return -1;

}

