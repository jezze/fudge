#include <abi.h>
#include <fudge.h>

void main(void)
{

    unsigned int index = 0;
    unsigned int count;
    struct record record;
    char num[8];

    call_open(CALL_PO);
    call_open(CALL_PW);
    call_seek(CALL_PW, index);
    call_read(CALL_PW, sizeof (struct record), &record);

    count = ascii_wzerovalue(num, 8, record.id, 16, 8, 0);

    call_write(CALL_PO, count, num);
    call_write(CALL_PO, 1, " ");

    count = ascii_wzerovalue(num, 8, record.size, 16, 8, 0);

    call_write(CALL_PO, count, num);
    call_write(CALL_PO, 5, " ../\n");

    while ((index = call_scan(CALL_PW, index)))
    {

        call_seek(CALL_PW, index);
        call_read(CALL_PW, sizeof (struct record), &record);

        count = ascii_wzerovalue(num, 8, record.id, 16, 8, 0);

        call_write(CALL_PO, count, num);
        call_write(CALL_PO, 1, " ");

        count = ascii_wzerovalue(num, 8, record.size, 16, 8, 0);

        call_write(CALL_PO, count, num);
        call_write(CALL_PO, 1, " ");
        call_write(CALL_PO, record.length, record.name);
        call_write(CALL_PO, 1, "\n");

    }

    call_close(CALL_PW);
    call_close(CALL_PO);

}

