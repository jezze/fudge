#include <abi.h>
#include <fudge.h>
#include <lib/file.h>

void main(void)
{

    unsigned int index = 0;
    struct record record;

    call_open(CALL_PO);
    call_open(CALL_PW);
    file_seekreadall(CALL_PW, &record, sizeof (struct record), index);
    file_writeall(CALL_PO, "../\n", 4);

    while ((index = call_scan(CALL_PW, index)))
    {

        file_seekreadall(CALL_PW, &record, sizeof (struct record), index);
        file_writeall(CALL_PO, record.name, record.length);
        file_writeall(CALL_PO, "\n", 1);

    }

    call_close(CALL_PW);
    call_close(CALL_PO);

}

