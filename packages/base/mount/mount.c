#include <fudge.h>

void main()
{

    call_open(FUDGE_DO, FUDGE_DR, 7, "system/");
    call_open(FUDGE_DI, FUDGE_DR, 18, "boot/mod/system.ko");
    call_mount(2, FUDGE_DO, FUDGE_DI);
    call_open(FUDGE_DO, FUDGE_DR, 5, "temp/");
    call_open(FUDGE_DI, FUDGE_DR, 16, "boot/mod/temp.ko");
    call_mount(3, FUDGE_DO, FUDGE_DI);

}

