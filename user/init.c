#include <fudge.h>

void main(int argc, char *argv[])
{

    call_load("/ramdisk/mod/nodefs.ko");
    call_load("/ramdisk/mod/uart.ko");
    call_load("/ramdisk/mod/vga.ko");
    call_load("/ramdisk/mod/pci.ko");
    call_load("/ramdisk/mod/pit.ko");
    call_load("/ramdisk/mod/rtc.ko");
    call_load("/ramdisk/mod/ps2.ko");
    call_load("/ramdisk/mod/tty.ko");
    call_load("/ramdisk/mod/bga.ko");

//    call_load("/ramdisk/mod/ata.ko");
//    call_load("/ramdisk/mod/mbr.ko");
//    call_load("/ramdisk/mod/ext2.ko");

    call_execute("/ramdisk/bin/shell", 0, 0);

    for (;;)
        call_halt();

}

