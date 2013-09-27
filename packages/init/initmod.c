#include <fudge.h>

static unsigned int nextline(unsigned int offset, unsigned int count, char *buffer)
{

    unsigned int i;

    for (i = offset; i < count; i++)
    {

        if (buffer[i] == '\n')
            return i - offset + 1;

    }

    return 0;

}

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

        for (loff = 0; (count2 = nextline(loff, count, buffer)); loff += count2)
        {

            call_open(CALL_I0, CALL_L0, count2 - 1, buffer + loff);
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

