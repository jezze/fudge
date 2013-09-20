#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];

    if (!call_open(CALL_L0, CALL_DR, 27, "system/nodefs/rtc_timestamp"))
        return;

    call_write(CALL_O0, 0, call_read(CALL_L0, 0, FUDGE_BSIZE, buffer), buffer);
    call_close(CALL_L0);

}

