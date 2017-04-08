#include <abi.h>
#include <fudge.h>

void main(void)
{

    struct record record;
    char num[FUDGE_NSIZE];

    file_open(CALL_PO);
    file_open(CALL_PW);
    file_writeall(CALL_PO, num, ascii_wzerovalue(num, FUDGE_NSIZE, 0, 16, 8, 0));
    file_writeall(CALL_PO, " ", 1);
    file_writeall(CALL_PO, num, ascii_wzerovalue(num, FUDGE_NSIZE, 0, 16, 8, 0));
    file_writeall(CALL_PO, " ", 1);
    file_writeall(CALL_PO, "../", 3);
    file_writeall(CALL_PO, "\n", 1);

    while (file_readall(CALL_PW, &record, sizeof (struct record)))
    {

        file_writeall(CALL_PO, num, ascii_wzerovalue(num, FUDGE_NSIZE, record.id, 16, 8, 0));
        file_writeall(CALL_PO, " ", 1);
        file_writeall(CALL_PO, num, ascii_wzerovalue(num, FUDGE_NSIZE, record.size, 16, 8, 0));
        file_writeall(CALL_PO, " ", 1);
        file_writeall(CALL_PO, record.name, record.length);
        file_writeall(CALL_PO, "\n", 1);
        file_step(CALL_PW);

    }

    file_close(CALL_PW);
    file_close(CALL_PO);

}

