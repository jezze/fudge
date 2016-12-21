#include <abi.h>
#include <fudge.h>

static void setpart0(void)
{

    struct ctrl_partsettings settings;

    ctrl_setpartsettings(&settings, 0, 2048 * 512, 40000 * 512);

    if (!file_walk(CALL_L0, "/system/part/part:0/ctrl"))
        return;

    file_open(CALL_L0);
    file_write(CALL_L0, &settings, sizeof (struct ctrl_partsettings));
    file_close(CALL_L0);


}

static void setpart1(void)
{

    struct ctrl_partsettings settings;

    ctrl_setpartsettings(&settings, 0, 40960 * 512, 1048575 * 512);

    if (!file_walk(CALL_L0, "/system/part/part:1/ctrl"))
        return;

    file_open(CALL_L0);
    file_write(CALL_L0, &settings, sizeof (struct ctrl_partsettings));
    file_close(CALL_L0);

}

void main(void)
{

    setpart0();
    setpart1();

}

