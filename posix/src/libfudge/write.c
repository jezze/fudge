#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include <call.h>

#undef errno
extern int errno;

int write(int file, char *ptr, int len)
{

    return call_write(file + 1, ptr, (unsigned int)len);

}

