#include <fudge.h>

void main(int argc, char *argv[])
{

    call_close(FILE_STDIN);
    call_close(FILE_STDOUT);

    unsigned int id = call_open(3, "/ramdisk/bin/load");

    call_open(FILE_STDIN, "/ramdisk/mod/nodefs.ko");
    call_execute(id, 0, 0);

    call_open(FILE_STDIN, "/ramdisk/mod/uart.ko");
    call_execute(id, 0, 0);

    call_open(FILE_STDIN, "/ramdisk/mod/vga.ko");
    call_execute(id, 0, 0);

    call_open(FILE_STDIN, "/ramdisk/mod/pci.ko");
    call_execute(id, 0, 0);

    call_open(FILE_STDIN, "/ramdisk/mod/pit.ko");
    call_execute(id, 0, 0);

    call_open(FILE_STDIN, "/ramdisk/mod/rtc.ko");
    call_execute(id, 0, 0);

    call_open(FILE_STDIN, "/ramdisk/mod/ps2.ko");
    call_execute(id, 0, 0);

    call_open(FILE_STDIN, "/ramdisk/mod/tty.ko");
    call_execute(id, 0, 0);

    call_open(FILE_STDIN, "/ramdisk/mod/bga.ko");
    call_execute(id, 0, 0);

    call_close(id);

    call_close(FILE_STDIN);
    call_close(FILE_STDOUT);

    unsigned int id2 = call_open(3, "/ramdisk/bin/shell");
    call_execute(id2, 0, 0);
    call_close(id2);

    for (;;);

}

