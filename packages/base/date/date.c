#include <fudge.h>

void main()
{

    char buffer[FUDGE_BSIZE];

    if (!call_open(3, 25, "/dev/nodefs/rtc_timestamp"))
        return;

    call_write(FUDGE_OUT, 0, call_read(3, 0, FUDGE_BSIZE, buffer), buffer);
    call_close(3);

}

