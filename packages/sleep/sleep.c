#include <fudge.h>

void main()
{

    char buffer[32];
    unsigned int count = call_read(CALL_I0, 0, 32, buffer);

    if (!count)
        return;

    if (!call_open(CALL_L0, CALL_DR, 18, "system/timer/clone"))
        return;

    call_open(CALL_L1, CALL_L0, 8, "../sleep");
    call_write(CALL_L1, 0, count, buffer);
    call_read(CALL_L1, 0, 32, buffer);
    call_close(CALL_L1);
    call_close(CALL_L0);

}

