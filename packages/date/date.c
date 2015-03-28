#include <abi.h>
#include <fudge.h>

void main()
{

    unsigned char date[FUDGE_BSIZE];
    unsigned char time[FUDGE_BSIZE];
    unsigned int woff = 0;

    if (!call_walk(CALL_L0, CALL_PR, 21, "system/clock/rtc/date"))
        return;

    if (!call_walk(CALL_L1, CALL_PR, 21, "system/clock/rtc/time"))
        return;

    call_open(CALL_PO);
    call_open(CALL_L0);

    woff += call_write(CALL_PO, woff, 1, call_read(CALL_L0, 0, 1, FUDGE_BSIZE, date), date);
    woff += call_write(CALL_PO, woff, 1, 1, " ");

    call_close(CALL_L0);
    call_open(CALL_L1);

    woff += call_write(CALL_PO, woff, 1, call_read(CALL_L1, 0, 1, FUDGE_BSIZE, time), time);
    woff += call_write(CALL_PO, woff, 1, 1, "\n");

    call_close(CALL_L1);
    call_close(CALL_PO);

}

