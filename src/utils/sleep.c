#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned char buffer[32];
    unsigned int count = file_read(CALL_PI, buffer, 32);

    if (!count)
        return;

    if (!file_walk(CALL_L0, "/system/timer/clone"))
        return;

    if (file_walkfrom(CALL_L1, CALL_L0, "../sleep"))
    {

        file_open(CALL_L1);
        file_writeall(CALL_L1, buffer, count);
        file_read(CALL_L1, buffer, 32);
        file_close(CALL_L1);

    }

}

