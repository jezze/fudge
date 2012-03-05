#include <fudge.h>

void main(int argc, char *argv[])
{

    call_load("/ramdisk/build/root/mod/nodefs.ko");
    call_load("/ramdisk/build/root/mod/uart.ko");
    call_load("/ramdisk/build/root/mod/vga.ko");
    call_load("/ramdisk/build/root/mod/pci.ko");
    call_load("/ramdisk/build/root/mod/pit.ko");
    call_load("/ramdisk/build/root/mod/rtc.ko");
    call_load("/ramdisk/build/root/mod/ps2.ko");
    call_load("/ramdisk/build/root/mod/tty.ko");
    call_execute("/ramdisk/build/root/bin/shell", 0, 0);

    for (;;)
        call_halt();

}

