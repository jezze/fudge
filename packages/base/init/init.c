#include <fudge.h>

void load_modules_core(unsigned int id)
{

    call_open(FILE_STDIN, 22, "/ramdisk/mod/nodefs.ko");
    call_execute(id);

    call_mount(FILE_STDIN, 3, 8, "/module/");

    call_open(FILE_STDIN, 19, "/ramdisk/mod/net.ko");
    call_execute(id);

    call_mount(FILE_STDIN, 4, 5, "/net/");

    call_open(FILE_STDIN, 19, "/ramdisk/mod/log.ko");
    call_execute(id);

    call_open(FILE_STDIN, 18, "/ramdisk/mod/io.ko");
    call_execute(id);

    call_open(FILE_STDIN, 21, "/ramdisk/mod/cpuid.ko");
    call_execute(id);

    call_open(FILE_STDIN, 19, "/ramdisk/mod/msr.ko");
    call_execute(id);

    call_open(FILE_STDIN, 20, "/ramdisk/mod/apic.ko");
    call_execute(id);

    call_open(FILE_STDIN, 19, "/ramdisk/mod/vga.ko");
    call_execute(id);

    call_open(FILE_STDIN, 19, "/ramdisk/mod/ps2.ko");
    call_execute(id);

    call_open(FILE_STDIN, 19, "/ramdisk/mod/tty.ko");
    call_execute(id);

    call_mount(FILE_STDIN, 5, 5, "/tty/");

}

void load_modules_extra(unsigned int id)
{

    call_open(FILE_STDIN, 20, "/ramdisk/mod/acpi.ko");
    call_execute(id);

    call_open(FILE_STDIN, 19, "/ramdisk/mod/fpu.ko");
    call_execute(id);

    call_open(FILE_STDIN, 19, "/ramdisk/mod/pci.ko");
    call_execute(id);

    call_open(FILE_STDIN, 19, "/ramdisk/mod/pit.ko");
    call_execute(id);

    call_open(FILE_STDIN, 19, "/ramdisk/mod/rtc.ko");
    call_execute(id);

    call_open(FILE_STDIN, 20, "/ramdisk/mod/uart.ko");
    call_execute(id);

}

void load_modules_testing(unsigned int id)
{

    /* Experimental */
    call_open(FILE_STDIN, 19, "/ramdisk/mod/ata.ko");
    call_execute(id);

    /* Experimental */
    call_open(FILE_STDIN, 19, "/ramdisk/mod/mbr.ko");
    call_execute(id);

    /* Experimental */
    call_open(FILE_STDIN, 20, "/ramdisk/mod/ext2.ko");
    call_execute(id);

    /* Does not work if device is non-existing */
    call_open(FILE_STDIN, 23, "/ramdisk/mod/rtl8139.ko");
    call_execute(id);

    /* Does not work if device is non-existing */
    call_open(FILE_STDIN, 19, "/ramdisk/mod/bga.ko");
    call_execute(id);

}

void load_modules()
{

    call_open(3, 17, "/ramdisk/bin/load");

    load_modules_core(3);
    load_modules_extra(3);

    call_close(3);

}

void start_shell()
{

    call_open(3, 8, "/tty/cwd");
    call_write(3, 0, 14, "/ramdisk/home/");
    call_close(3);

    call_open(FILE_STDIN, 10, "/tty/stdin");
    call_open(FILE_STDOUT, 11, "/tty/stdout");

    call_write(FILE_STDOUT, 0, 23, "FUDGE OPERATING SYSTEM\n");
    call_write(FILE_STDOUT, 0, 12, "Build date: ");
    call_write(FILE_STDOUT, 0, 11, __DATE__);
    call_write(FILE_STDOUT, 0, 1, " ");
    call_write(FILE_STDOUT, 0, 8, __TIME__);
    call_write(FILE_STDOUT, 0, 2, "\n\n");
    call_write(FILE_STDOUT, 0, 53, "Write `echo <help.txt` for a short list if commands\n\n");

    call_open(3, 18, "/ramdisk/bin/shell");
    call_execute(3);
    call_close(3);

}

void main()
{

    load_modules();
    start_shell();

    for (;;);

}

