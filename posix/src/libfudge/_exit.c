#include <_ansi.h>
#include <_syslist.h>
#include <limits.h>

void _exit(int rc)
{

    /* Default stub just causes a divide by 0 exception.  */
    int x = rc / INT_MAX;
    x = 4 / x;

    for (;;);

}
