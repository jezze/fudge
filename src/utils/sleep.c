#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[32];
    unsigned int count = file_read(CALL_PI, buffer, 32);

    if (!count)
        return;

    if (!call_walk(CALL_L0, CALL_PR, 18, "system/timer/clone"))
        return;

    if (call_walk(CALL_L1, CALL_L0, 8, "../sleep"))
    {

        file_open(CALL_L1);
        file_writeall(CALL_L1, buffer, count);
        file_read(CALL_L1, buffer, 32);
        file_close(CALL_L1);

    }

}

