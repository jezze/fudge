#include <abi.h>
#include <fudge.h>

void main()
{

    struct record records[64];
    unsigned int count, roff, woff = 0;

    call_open(CALL_PO);
    call_open(CALL_PW);

    for (roff = 0; (count = call_read(CALL_PW, roff, sizeof (struct record) * 64, records)); roff += count)
    {

        unsigned int i;

        for (i = 0; i < count; i++)
        {

            woff += call_write(CALL_PO, woff, records[i].length, records[i].name);
            woff += call_write(CALL_PO, woff, 1, "\n");

        }

    }

    call_close(CALL_PW);
    call_close(CALL_PO);

}

