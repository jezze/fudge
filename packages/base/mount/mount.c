#include <fudge.h>

void main()
{

    call_open(FUDGE_OUT, FUDGE_ROOT, 7, "system/");
    call_open(FUDGE_IN, FUDGE_ROOT, 18, "boot/mod/system.ko");
    call_mount(2, FUDGE_OUT, FUDGE_IN);
    call_open(FUDGE_OUT, FUDGE_ROOT, 5, "temp/");
    call_open(FUDGE_IN, FUDGE_ROOT, 16, "boot/mod/temp.ko");
    call_mount(3, FUDGE_OUT, FUDGE_IN);

}

