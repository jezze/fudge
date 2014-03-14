#include <fudge.h>

static void parse()
{

    char buffer[FUDGE_BSIZE];
    unsigned int count, roff, loff;

    for (roff = 0; (count = call_read(CALL_I0, roff, FUDGE_BSIZE, buffer)); roff += loff)
    {

        unsigned int count2;

        for (loff = 0; (count2 = memory_findbyte(buffer + loff, count - loff, '\n')); loff += count2 + 1)
        {

            if (call_walk(CALL_I1, CALL_L1, count2, buffer + loff))
            {

                call_open(CALL_I1);
                call_spawn(CALL_L0);
                call_close(CALL_I1);

            }

        }

        if (!loff)
            return;

    }

}

void main()
{

    if (!call_walk(CALL_O1, CALL_O0, 0, 0))
        return;

    call_open(CALL_O1);

    if (call_walk(CALL_L0, CALL_DR, 11, "bin/modload"))
    {

        call_open(CALL_L0);

        if (call_walk(CALL_L1, CALL_DR, 9, "boot/mod/"))
        {

            call_open(CALL_L1);
            parse();
            call_close(CALL_L1);

        }

        call_close(CALL_L0);

    }

    call_close(CALL_O1);

}

