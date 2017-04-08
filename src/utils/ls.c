#include <abi.h>
#include <fudge.h>

void main(void)
{

    struct record record;

    file_open(CALL_PO);
    file_open(CALL_PW);
    file_writeall(CALL_PO, "../\n", 4);

    while (file_readall(CALL_PW, &record, sizeof (struct record)))
    {

        file_writeall(CALL_PO, record.name, record.length);
        file_writeall(CALL_PO, "\n", 1);
        file_step(CALL_PW);

    }

    file_close(CALL_PW);
    file_close(CALL_PO);

}

