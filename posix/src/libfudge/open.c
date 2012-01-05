#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include <call.h>

#undef errno
extern int errno;

int open(char *file, int flags, int mode)
{

    return call_open("initrd", file) + 1;

}

