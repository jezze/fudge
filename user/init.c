#include <fudge.h>

void main(int argc, char *argv[])
{

    unsigned int fd;

    fd = call_open("/ramdisk/mod/nodefs.ko");
    call_load(fd);
    call_close(fd);

    fd = call_open("/ramdisk/mod/uart.ko");
    call_load(fd);
    call_close(fd);

    fd = call_open("/ramdisk/mod/vga.ko");
    call_load(fd);
    call_close(fd);

    fd = call_open("/ramdisk/mod/pci.ko");
    call_load(fd);
    call_close(fd);

    fd = call_open("/ramdisk/mod/pit.ko");
    call_load(fd);
    call_close(fd);

    fd = call_open("/ramdisk/mod/rtc.ko");
    call_load(fd);
    call_close(fd);

    fd = call_open("/ramdisk/mod/ps2.ko");
    call_load(fd);
    call_close(fd);

    fd = call_open("/ramdisk/mod/tty.ko");
    call_load(fd);
    call_close(fd);

    fd = call_open("/ramdisk/mod/bga.ko");
    call_load(fd);
    call_close(fd);

    call_execute("/ramdisk/bin/shell", 0, 0);

    for (;;);

}

