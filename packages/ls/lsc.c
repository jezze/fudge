#include <abi.h>
#include <fudge.h>

void main()
{

    struct record records[64];
    unsigned int count, roff, woff = 0;

    call_open(CALL_PO);
    call_open(CALL_PW);

    for (roff = 0; (count = call_read(CALL_PW, roff, sizeof (struct record), 64, records)); roff += count)
    {

        unsigned int i;

        for (i = 0; i < count; i++)
        {

            unsigned char num[32];

            woff += call_write(CALL_PO, woff, ascii_wzerovalue(num, 32, records[i].size, 16, 8, 0), 1, num);
            woff += call_write(CALL_PO, woff, 2, 1, "  ");
            woff += call_write(CALL_PO, woff, records[i].length, 1, records[i].name);
            woff += call_write(CALL_PO, woff, 1, 1, "\n");

        }

    }

    call_close(CALL_PW);
    call_close(CALL_PO);

}

