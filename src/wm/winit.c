#include <abi.h>
#include <fudge.h>

void main(void)
{

    struct ctrl_videosettings settings;

    ctrl_setvideosettings(&settings, 1920, 1080, 32);

    if (!file_walkfrom(CALL_L0, CALL_PO, "ctrl"))
        return;

    file_open(CALL_L0);
    file_writeall(CALL_L0, &settings, sizeof (struct ctrl_videosettings));
    file_close(CALL_L0);

}

