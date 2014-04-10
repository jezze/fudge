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

            if (!call_walk(CALL_I1, CALL_L1, count2, buffer + loff))
                continue;

            call_spawn(CALL_L0);

        }

        if (!loff)
            return;

    }

}

void main()
{

    if (!call_walk(CALL_L0, CALL_DR, 11, "bin/modload"))
        return;

    if (!call_walk(CALL_L1, CALL_DR, 9, "boot/mod/"))
        return;

    parse();

}

