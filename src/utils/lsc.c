#include <abi.h>
#include <fudge.h>
#include <lib/file.h>

void main(void)
{

    unsigned int count;
    struct record record;
    char num[8];

    call_open(CALL_PO);
    call_open(CALL_PW);

    count = ascii_wzerovalue(num, 8, 0, 16, 8, 0);

    file_writeall(CALL_PO, num, count);
    file_writeall(CALL_PO, " ", 1);

    count = ascii_wzerovalue(num, 8, 0, 16, 8, 0);

    file_writeall(CALL_PO, num, count);
    file_writeall(CALL_PO, " ", 1);
    file_writeall(CALL_PO, "../", 3);
    file_writeall(CALL_PO, "\n", 1);

    while (file_readall(CALL_PW, &record, sizeof (struct record)))
    {

        count = ascii_wzerovalue(num, 8, record.id, 16, 8, 0);

        file_writeall(CALL_PO, num, count);
        file_writeall(CALL_PO, " ", 1);

        count = ascii_wzerovalue(num, 8, record.size, 16, 8, 0);

        file_writeall(CALL_PO, num, count);
        file_writeall(CALL_PO, " ", 1);
        file_writeall(CALL_PO, record.name, record.length);
        file_writeall(CALL_PO, "\n", 1);

    }

    call_close(CALL_PW);
    call_close(CALL_PO);

}

