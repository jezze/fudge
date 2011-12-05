#include <fudge.h>

void main(int argc, char *argv[])
{

    for (;;)
    {

        call_load("uart.ko");
        call_load("vga.ko");
        call_load("pci.ko");
        call_load("ata.ko");
        call_load("pit.ko");
        call_load("rtc.ko");
        call_load("ps2_kbd.ko");
        call_load("ps2_mouse.ko");
        call_load("rtl8139.ko");
        call_load("tty.ko");
        call_execute("shell", 0, 0);

    }

}

