#include <abi.h>
#include <fudge.h>

void main(void)
{

    struct ctrl_partsettings settings;

    ctrl_setpartsettings(&settings, 0, 2048, 40000);

    if (!file_walk(CALL_L0, "/system/part/clone/ctrl"))
        return;

    file_open(CALL_L0);
    file_write(CALL_L0, &settings, sizeof (struct ctrl_partsettings));
    file_close(CALL_L0);

}

