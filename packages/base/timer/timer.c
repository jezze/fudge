#include <fudge.h>

void main()
{

    char buffer[FUDGE_BSIZE];

    if (!call_open(3, FUDGE_ROOT, 25, "system/nodefs/pit_jiffies"))
        return;

    call_write(FUDGE_OUT, 0, call_read(3, 0, FUDGE_BSIZE, buffer), buffer);
    call_close(3);

}

