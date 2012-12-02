#include <fudge.h>

void load_modules_core(unsigned int id)
{

    call_open(FILE_STDIN, 22, "/ramdisk/mod/nodefs.ko");
    call_spawn(id);

    call_open(11, 8, "/nodefs/");
    call_mount(4, 11, FILE_STDIN);

    call_open(FILE_STDIN, 21, "/ramdisk/mod/video.ko");
    call_spawn(id);

    call_open(11, 7, "/video/");
    call_mount(5, 11, FILE_STDIN);

    call_open(FILE_STDIN, 19, "/ramdisk/mod/net.ko");
    call_spawn(id);

    call_open(FILE_STDIN, 21, "/ramdisk/mod/block.ko");
    call_spawn(id);

    call_open(FILE_STDIN, 18, "/ramdisk/mod/io.ko");
    call_spawn(id);

    call_open(FILE_STDIN, 21, "/ramdisk/mod/cpuid.ko");
    call_spawn(id);

    call_open(FILE_STDIN, 19, "/ramdisk/mod/msr.ko");
    call_spawn(id);

    call_open(FILE_STDIN, 20, "/ramdisk/mod/apic.ko");
    call_spawn(id);

    call_open(FILE_STDIN, 19, "/ramdisk/mod/vga.ko");
    call_spawn(id);

    call_open(FILE_STDIN, 19, "/ramdisk/mod/ps2.ko");
    call_spawn(id);

    call_open(FILE_STDIN, 19, "/ramdisk/mod/tty.ko");
    call_spawn(id);

    call_open(11, 5, "/tty/");
    call_mount(3, 11, FILE_STDIN);

}

void load_modules_extra(unsigned int id)
{

/*
    call_open(FILE_STDIN, 20, "/ramdisk/mod/acpi.ko");
    call_spawn(id);
*/

    call_open(FILE_STDIN, 19, "/ramdisk/mod/fpu.ko");
    call_spawn(id);

    call_open(FILE_STDIN, 19, "/ramdisk/mod/pci.ko");
    call_spawn(id);

    call_open(FILE_STDIN, 19, "/ramdisk/mod/pit.ko");
    call_spawn(id);

    call_open(FILE_STDIN, 19, "/ramdisk/mod/rtc.ko");
    call_spawn(id);

    call_open(FILE_STDIN, 20, "/ramdisk/mod/uart.ko");
    call_spawn(id);

    call_open(FILE_STDIN, 19, "/ramdisk/mod/arp.ko");
    call_spawn(id);

    call_open(FILE_STDIN, 20, "/ramdisk/mod/ipv4.ko");
    call_spawn(id);

}

void load_modules_testing(unsigned int id)
{

    /* Experimental */
    call_open(FILE_STDIN, 19, "/ramdisk/mod/ata.ko");
    call_spawn(id);

    /* Experimental */
    call_open(FILE_STDIN, 20, "/ramdisk/mod/ext2.ko");
    call_spawn(id);

    /* Does not work if device is non-existing */
    call_open(FILE_STDIN, 23, "/ramdisk/mod/rtl8139.ko");
    call_spawn(id);

    /* Does not work if device is non-existing */
    call_open(FILE_STDIN, 19, "/ramdisk/mod/bga.ko");
    call_spawn(id);

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

    call_open(FILE_STDIN, 18, "/nodefs/ps2_buffer");
    call_open(FILE_STDOUT, 13, "/video/0/data");

    call_open(3, 17, "/ramdisk/bin/motd");
    call_spawn(3);
    call_close(3);

    call_open(3, 18, "/ramdisk/bin/shell");
    call_execute(3);

}

void main()
{

    load_modules();
    start_shell();

}

