#include <abi.h>
#include <fudge.h>

void main(void)
{
/*
    char buffer[FUDGE_BSIZE];
    unsigned int count0, count1, roff0, roff1;

    if (!call_walk(CALL_CP, CALL_PR, 11, "bin/modload"))
        return;

    if (!call_walk(CALL_L1, CALL_PR, 9, "boot/mod/"))
        return;

    call_open(CALL_P0);

    for (roff0 = 0; (count0 = call_read(CALL_P0, roff0, 1, FUDGE_BSIZE, buffer)); roff0 += roff1)
    {

        for (roff1 = 0; (count1 = memory_findbyte(buffer + roff1, count0 - roff1, '\n')); roff1 += count1)
        {

            if (!call_walk(CALL_C0, CALL_L1, count1 - 1, buffer + roff1))
                continue;

            call_spawn();

        }

    }

    call_close(CALL_P0);
*/
}

