#include <fudge.h>

void load_modules()
{

    unsigned int id;

    id = call_open(3, "/ramdisk/bin/load");

    call_open(FILE_STDIN, "/ramdisk/mod/nodefs.ko");
    call_execute(id);

    call_open(FILE_STDIN, "/ramdisk/mod/log.ko");
    call_execute(id);

    call_open(FILE_STDIN, "/ramdisk/mod/uart.ko");
    call_execute(id);

    call_open(FILE_STDIN, "/ramdisk/mod/vga.ko");
    call_execute(id);

    call_open(FILE_STDIN, "/ramdisk/mod/pci.ko");
    call_execute(id);

    call_open(FILE_STDIN, "/ramdisk/mod/cpuid.ko");
    call_execute(id);

    call_open(FILE_STDIN, "/ramdisk/mod/acpi.ko");
    call_execute(id);

    call_open(FILE_STDIN, "/ramdisk/mod/pit.ko");
    call_execute(id);

    call_open(FILE_STDIN, "/ramdisk/mod/rtc.ko");
    call_execute(id);

    call_open(FILE_STDIN, "/ramdisk/mod/ps2.ko");
    call_execute(id);

    call_open(FILE_STDIN, "/ramdisk/mod/tty.ko");
    call_execute(id);

    /*

    call_open(FILE_STDIN, "/ramdisk/mod/ata.ko");
    call_execute(id);

    call_open(FILE_STDIN, "/ramdisk/mod/mbr.ko");
    call_execute(id);

    call_open(FILE_STDIN, "/ramdisk/mod/ext2.ko");
    call_execute(id);

    call_open(FILE_STDIN, "/ramdisk/mod/rtl8139.ko");
    call_execute(id);

    call_open(FILE_STDIN, "/ramdisk/mod/bga.ko");
    call_execute(id);

    */

    call_close(FILE_STDIN);
    call_close(id);

}

void main()
{

    unsigned int id;

    load_modules();

    id = call_open(3, "/ramdisk/bin/shell");
    call_execute(id);
    call_close(id);

    for (;;);

}

