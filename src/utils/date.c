#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char date[FUDGE_BSIZE];
    unsigned char time[FUDGE_BSIZE];

    if (!call_walk(CALL_L0, CALL_PR, 21, "system/clock/rtc/date"))
        return;

    if (!call_walk(CALL_L1, CALL_PR, 21, "system/clock/rtc/time"))
        return;

    call_open(CALL_PO);
    call_open(CALL_L0);
    call_write(CALL_PO, call_read(CALL_L0, FUDGE_BSIZE, date), date);
    call_write(CALL_PO, 1, " ");
    call_close(CALL_L0);
    call_open(CALL_L1);
    call_write(CALL_PO, call_read(CALL_L1, FUDGE_BSIZE, time), time);
    call_write(CALL_PO, 1, "\n");
    call_close(CALL_L1);
    call_close(CALL_PO);

}

