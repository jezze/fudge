#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char date[FUDGE_BSIZE];
    unsigned char time[FUDGE_BSIZE];

    if (!file_walk(CALL_L0, "/system/clock/rtc/date"))
        return;

    if (!file_walk(CALL_L1, "/system/clock/rtc/time"))
        return;

    file_open(CALL_PO);
    file_open(CALL_L0);
    file_writeall(CALL_PO, date, file_read(CALL_L0, date, FUDGE_BSIZE));
    file_writeall(CALL_PO, " ", 1);
    file_close(CALL_L0);
    file_open(CALL_L1);
    file_writeall(CALL_PO, time, file_read(CALL_L1, time, FUDGE_BSIZE));
    file_writeall(CALL_PO, "\n", 1);
    file_close(CALL_L1);
    file_close(CALL_PO);

}

