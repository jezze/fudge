#include <abi.h>
#include <fudge.h>

void main(void)
{

    struct record record;

    file_open(FILE_PO);
    file_open(FILE_PW);
    file_writeall(FILE_PO, "..\n", 3);

    while (file_readall(FILE_PW, &record, sizeof (struct record)))
    {

        file_writeall(FILE_PO, record.name, record.length);
        file_writeall(FILE_PO, "\n", 1);
        file_step(FILE_PW);

    }

    file_close(FILE_PW);
    file_close(FILE_PO);

}

