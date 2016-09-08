#include <abi.h>
#include <fudge.h>

void main(void)
{

    struct ctrl_consettings settings;

    file_write(CALL_PO, "hej\n", 4);

    if (!file_walk(CALL_L1, "ctrl"))
        return;

    file_write(CALL_PO, "hej\n", 4);

    file_open(CALL_L1);
    file_readall(CALL_L1, &settings, sizeof (struct ctrl_consettings));

    settings.interface = 0;
    settings.protocol = 0x11;

    file_writeall(CALL_L1, &settings, sizeof (struct ctrl_consettings));
    file_close(CALL_L1);

    file_write(CALL_PO, "hej\n", 4);

}

