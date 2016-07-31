#include <abi.h>
#include <fudge.h>

void main(void)
{

    struct record record;

    call_open(CALL_PO);
    call_open(CALL_PW);
    file_writeall(CALL_PO, "../\n", 4);

    while (file_readall(CALL_PW, &record, sizeof (struct record)))
    {

        file_writeall(CALL_PO, record.name, record.length);
        file_writeall(CALL_PO, "\n", 1);

    }

    call_close(CALL_PW);
    call_close(CALL_PO);

}

