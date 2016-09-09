#include <abi.h>
#include <fudge.h>

void main(void)
{

    struct ctrl_consettings settings;
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    ctrl_setconsettings(&settings, 0, 0x11, 8080);

    if (!file_walk(CALL_L1, "ctrl"))
        return;

    if (!file_walk(CALL_L2, "data"))
        return;

    file_open(CALL_L1);
    file_writeall(CALL_L1, &settings, sizeof (struct ctrl_consettings));
    file_close(CALL_L1);
    file_open(CALL_PO);
    file_open(CALL_L2);

    while ((count = file_read(CALL_L2, buffer, FUDGE_BSIZE)))
        file_writeall(CALL_PO, buffer, count);

    file_close(CALL_L2);
    file_close(CALL_PO);

}

