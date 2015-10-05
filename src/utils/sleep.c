#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[32];
    unsigned int count = call_read(CALL_PI, 32, buffer);

    if (!count)
        return;

    if (!call_walk(CALL_L0, CALL_PR, 18, "system/timer/clone"))
        return;

    if (call_walk(CALL_L1, CALL_L0, 8, "../sleep"))
    {

        call_open(CALL_L1);
        call_write(CALL_L1, count, buffer);
        call_read(CALL_L1, 32, buffer);
        call_close(CALL_L1);

    }

}

