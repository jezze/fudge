#include <fudge.h>

void main()
{

    call_open(FUDGE_CWD, FUDGE_ROOT, 5, "home/");
    call_open(FUDGE_IN, FUDGE_ROOT, 17, "config/base.slang");
    call_open(3, FUDGE_ROOT, 9, "bin/slang");
    call_spawn(3);
    call_close(3);
    call_close(FUDGE_IN);
    call_open(3, FUDGE_ROOT, 7, "system/");
    call_open(4, FUDGE_ROOT, 18, "boot/mod/system.ko");
    call_mount(2, 3, 4);
    call_close(4);
    call_close(3);
    call_open(3, FUDGE_ROOT, 5, "temp/");
    call_open(4, FUDGE_ROOT, 16, "boot/mod/temp.ko");
    call_mount(3, 3, 4);
    call_close(4);
    call_close(3);
    call_open(FUDGE_IN, FUDGE_ROOT, 19, "system/kbd/ps2/data");
    call_open(FUDGE_OUT, FUDGE_ROOT, 24, "system/terminal/vga/data");
    call_open(3, FUDGE_ROOT, 8, "bin/motd");
    call_spawn(3);
    call_close(3);
    call_open(3, FUDGE_ROOT, 9, "bin/shell");
    call_execute(3);
    call_close(3);

}

