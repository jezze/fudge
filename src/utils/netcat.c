#include <abi.h>
#include <fudge.h>

void main(void)
{

    struct ctrl_consettings settings;
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    ctrl_setconsettings(&settings, 0, 0x11, 8080);

    if (!file_walk(CALL_L1, "/system/con/clone/"))
        return;

    if (!file_walkfrom(CALL_L2, CALL_L1, "ctrl"))
        return;

    if (!file_walkfrom(CALL_L3, CALL_L1, "data"))
        return;

    file_open(CALL_L2);
    file_writeall(CALL_L2, &settings, sizeof (struct ctrl_consettings));
    file_close(CALL_L2);
    file_open(CALL_PO);
    file_open(CALL_L3);
    file_writeall(CALL_PO, "== Listening on UDP port 8080 ==\n\n", 34);

    while ((count = file_read(CALL_L3, buffer, FUDGE_BSIZE)))
        file_writeall(CALL_PO, buffer, count);

    file_close(CALL_L3);
    file_close(CALL_PO);

}

