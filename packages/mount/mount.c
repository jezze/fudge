#include <fudge.h>

void main()
{

    call_open(CALL_DO, CALL_DR, 7, "system/");
    call_open(CALL_DI, CALL_DR, 18, "boot/mod/system.ko");
    call_mount(2, CALL_DO, CALL_DI);
    call_open(CALL_DO, CALL_DR, 5, "temp/");
    call_open(CALL_DI, CALL_DR, 16, "boot/mod/temp.ko");
    call_mount(3, CALL_DO, CALL_DI);

}

