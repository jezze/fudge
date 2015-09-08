#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned int index = 0;
    struct record record;

    call_open(CALL_PO);
    call_open(CALL_PW);
    call_seek(CALL_PW, index);
    call_read(CALL_PW, sizeof (struct record), &record);
    call_write(CALL_PO, 3, "../");
    call_write(CALL_PO, 1, "\n");

    while ((index = call_scan(CALL_PW, index)))
    {

        call_seek(CALL_PW, index);
        call_read(CALL_PW, sizeof (struct record), &record);
        call_write(CALL_PO, record.length, record.name);
        call_write(CALL_PO, 1, "\n");

    }

    call_close(CALL_PW);
    call_close(CALL_PO);

}

