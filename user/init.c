#include <fudge.h>

void main(int argc, char *argv[])
{

    call_load("mod/nodefs.ko");
    call_load("mod/uart.ko");
    call_load("mod/vga.ko");
    call_load("mod/pci.ko");
    call_load("mod/pit.ko");
    call_load("mod/rtc.ko");
    call_load("mod/ps2.ko");
    call_load("mod/tty.ko");
    call_execute("bin/shell", 0, 0);

    for (;;);

}

