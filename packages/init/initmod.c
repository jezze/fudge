#include <fudge.h>

void main()
{

    char buffer[FUDGE_BSIZE];
    unsigned int count, roff, loff;

    call_open(CALL_E0, CALL_DR, 11, "bin/modload");
    call_open(CALL_L0, CALL_DR, 9, "boot/mod/");
    call_open(CALL_L1, CALL_I0, 0, 0);

    for (roff = 0; (count = call_read(CALL_L1, roff, FUDGE_BSIZE, buffer)); roff += loff)
    {

        unsigned int count2;

        for (loff = 0; (count2 = memory_findbyte(buffer + loff, count - loff, '\n')); loff += count2 + 1)
        {

            call_open(CALL_I0, CALL_L0, count2, buffer + loff);
            call_spawn(CALL_E0);

        }

        if (!loff)
            return;

    }

    call_open(CALL_I0, CALL_L1, 0, 0);
    call_close(CALL_E0);
    call_close(CALL_L0);
    call_close(CALL_L1);

}

