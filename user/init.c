#include <fudge.h>

void main(int argc, char *argv[])
{

    call_load("build/root/mod/nodefs.ko");
    call_load("build/root/mod/uart.ko");
    call_load("build/root/mod/vga.ko");
    call_load("build/root/mod/pci.ko");
    call_load("build/root/mod/pit.ko");
    call_load("build/root/mod/rtc.ko");
    call_load("build/root/mod/ps2.ko");
    call_load("build/root/mod/tty.ko");
    call_execute("build/root/bin/shell", 0, 0);

    for (;;)
        call_halt();

}

