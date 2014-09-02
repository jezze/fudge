#include <fudge.h>

void main()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count, roff, woff = 0;

    call_open(CALL_O0);
    call_open(CALL_I0);

    for (roff = 0; (count = call_read(CALL_I0, roff, FUDGE_BSIZE, buffer)); roff += count)
    {

        unsigned int i;

        for (i = 0; i < count; i++)
        {

            unsigned char num[32];

            woff += call_write(CALL_O0, woff, ascii_wzerovalue(num, 32, buffer[i], 16, 2, 0), num);
            woff += call_write(CALL_O0, woff, 2, "  ");

        }

    }

    call_close(CALL_I0);
    call_close(CALL_O0);

}

