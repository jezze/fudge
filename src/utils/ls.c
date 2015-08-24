#include <abi.h>
#include <fudge.h>

void main(void)
{

    struct record records[64];
    unsigned int count;

    call_open(CALL_PO);
    call_open(CALL_PW);

    while ((count = call_read(CALL_PW, sizeof (struct record) * 64, records)))
    {

        unsigned int i;

        for (i = 0; i < count; i++)
        {

            call_write(CALL_PO, records[i].length, records[i].name);
            call_write(CALL_PO, 1, "\n");

        }

    }

    call_close(CALL_PW);
    call_close(CALL_PO);

}

