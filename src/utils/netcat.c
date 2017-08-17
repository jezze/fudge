#include <abi.h>
#include <fudge.h>

void main(void)
{

    struct ctrl_consettings settings;
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    ctrl_setconsettings(&settings, 0, 0x11, 8080);

    if (!file_walk(FILE_L1, "/system/con/clone"))
        return;

    if (!file_walkfrom(FILE_L2, FILE_L1, "ctrl"))
        return;

    if (!file_walkfrom(FILE_L3, FILE_L1, "data"))
        return;

    file_open(FILE_L2);
    file_writeall(FILE_L2, &settings, sizeof (struct ctrl_consettings));
    file_close(FILE_L2);
    file_open(FILE_PO);
    file_open(FILE_L3);
    file_writeall(FILE_PO, "IP: 10.0.5.5\n", 13);
    file_writeall(FILE_PO, "Protocol: UDP\n", 14);
    file_writeall(FILE_PO, "Port: 8080\n", 11);
    file_writeall(FILE_PO, "Listening...\n\n", 14);

    while ((count = file_read(FILE_L3, buffer, FUDGE_BSIZE)))
    {

        file_writeall(FILE_PO, buffer, count);
        file_writeall(FILE_L3, "HELLO", 5);

    }

    file_close(FILE_L3);
    file_close(FILE_PO);

}

