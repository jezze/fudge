#include <fudge.h>

void load_modules_core(unsigned int id)
{

    call_open(FILE_STDIN, 0, "/ramdisk/mod/nodefs.ko");
    call_execute(id);

    call_open(FILE_STDIN, 0, "/ramdisk/mod/log.ko");
    call_execute(id);

    call_open(FILE_STDIN, 0, "/ramdisk/mod/cpuid.ko");
    call_execute(id);

    call_open(FILE_STDIN, 0, "/ramdisk/mod/msr.ko");
    call_execute(id);

    call_open(FILE_STDIN, 0, "/ramdisk/mod/apic.ko");
    call_execute(id);

    call_open(FILE_STDIN, 0, "/ramdisk/mod/vga.ko");
    call_execute(id);

    call_open(FILE_STDIN, 0, "/ramdisk/mod/ps2.ko");
    call_execute(id);

    call_open(FILE_STDIN, 0, "/ramdisk/mod/tty.ko");
    call_execute(id);

}

void load_modules_extra(unsigned int id)
{

    call_open(FILE_STDIN, 0, "/ramdisk/mod/uart.ko");
    call_execute(id);

    call_open(FILE_STDIN, 0, "/ramdisk/mod/fpu.ko");
    call_execute(id);

    call_open(FILE_STDIN, 0, "/ramdisk/mod/pci.ko");
    call_execute(id);

    call_open(FILE_STDIN, 0, "/ramdisk/mod/pit.ko");
    call_execute(id);

    call_open(FILE_STDIN, 0, "/ramdisk/mod/rtc.ko");
    call_execute(id);

}

void load_modules_testing(unsigned int id)
{

    call_open(FILE_STDIN, 0, "/ramdisk/mod/ata.ko");
    call_execute(id);

    call_open(FILE_STDIN, 0, "/ramdisk/mod/mbr.ko");
    call_execute(id);

    call_open(FILE_STDIN, 0, "/ramdisk/mod/ext2.ko");
    call_execute(id);

    call_open(FILE_STDIN, 0, "/ramdisk/mod/rtl8139.ko");
    call_execute(id);

    call_open(FILE_STDIN, 0, "/ramdisk/mod/bga.ko");
    call_execute(id);

    call_open(FILE_STDIN, 0, "/ramdisk/mod/acpi.ko");
    call_execute(id);

}

void load_modules()
{

    unsigned int id;

    id = call_open(3, 0, "/ramdisk/bin/load");

    load_modules_core(id);
    load_modules_extra(id);
    /* load_modules_testing(id); */

    call_close(FILE_STDIN);
    call_close(id);

}

void main()
{

    unsigned int id;

    load_modules();

    id = call_open(3, 0, "/ramdisk/bin/shell");
    call_execute(id);
    call_close(id);

    for (;;);

}

