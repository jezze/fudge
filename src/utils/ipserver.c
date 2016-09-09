#include <abi.h>
#include <fudge.h>

void main(void)
{

    struct ctrl_consettings settings;

    ctrl_setconsettings(&settings, 0, 0x11, 8080);

    if (!file_walk(CALL_L1, "ctrl"))
        return;

    file_open(CALL_L1);
    file_writeall(CALL_L1, &settings, sizeof (struct ctrl_consettings));
    file_close(CALL_L1);

}

