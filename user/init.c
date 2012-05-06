#include <fudge.h>

void main(int argc, char *argv[])
{

    unsigned int id;
    unsigned int id2;

    call_close(FILE_STDIN);
    call_close(FILE_STDOUT);

    id = call_open("/ramdisk/bin/load");
    id2 = call_open("/ramdisk/bin/load");
    call_close(id);

    id = call_open("/ramdisk/mod/nodefs.ko");
    call_execute(id2, 0, 0);
    call_close(id);

    id = call_open("/ramdisk/mod/uart.ko");
    call_execute(id2, 0, 0);
    call_close(id);

    id = call_open("/ramdisk/mod/vga.ko");
    call_execute(id2, 0, 0);
    call_close(id);

    id = call_open("/ramdisk/mod/pci.ko");
    call_execute(id2, 0, 0);
    call_close(id);

    id = call_open("/ramdisk/mod/pit.ko");
    call_execute(id2, 0, 0);
    call_close(id);

    id = call_open("/ramdisk/mod/rtc.ko");
    call_execute(id2, 0, 0);
    call_close(id);

    id = call_open("/ramdisk/mod/ps2.ko");
    call_execute(id2, 0, 0);
    call_close(id);

    id = call_open("/ramdisk/mod/tty.ko");
    call_execute(id2, 0, 0);
    call_close(id);

    id = call_open("/ramdisk/mod/bga.ko");
    call_execute(id2, 0, 0);
    call_close(id);

    call_close(FILE_STDIN);
    call_close(FILE_STDOUT);
    call_close(id2);

    id = call_open("/ramdisk/bin/shell");
    call_execute(id, 0, 0);
    call_close(id);

    for (;;);

}

