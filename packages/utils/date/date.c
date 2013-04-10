#include <fudge.h>

void main()
{

    char buffer[FUDGE_BSIZE];

    if (!call_open(3, FUDGE_DR, 27, "system/nodefs/rtc_timestamp"))
        return;

    call_write(FUDGE_DO, 0, call_read(3, 0, FUDGE_BSIZE, buffer), buffer);
    call_close(3);

}

