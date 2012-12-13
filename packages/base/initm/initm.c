#include <fudge.h>

void load_modules_core(unsigned int id)
{

    call_open(FUDGE_IN, FUDGE_ROOT, 22, "/ramdisk/mod/system.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_ROOT, 22, "/ramdisk/mod/nodefs.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_ROOT, 21, "/ramdisk/mod/video.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_ROOT, 19, "/ramdisk/mod/net.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_ROOT, 21, "/ramdisk/mod/block.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_ROOT, 18, "/ramdisk/mod/io.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_ROOT, 21, "/ramdisk/mod/cpuid.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_ROOT, 19, "/ramdisk/mod/msr.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_ROOT, 19, "/ramdisk/mod/pic.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_ROOT, 19, "/ramdisk/mod/vga.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_ROOT, 19, "/ramdisk/mod/ps2.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_ROOT, 19, "/ramdisk/mod/tty.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

}

void load_modules_extra(unsigned int id)
{

/*
    call_open(FUDGE_IN, FUDGE_ROOT, 20, "/ramdisk/mod/acpi.ko");
    call_spawn(id);
    call_close(FUDGE_IN);
*/

    call_open(FUDGE_IN, FUDGE_ROOT, 19, "/ramdisk/mod/fpu.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_ROOT, 19, "/ramdisk/mod/pci.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_ROOT, 19, "/ramdisk/mod/pit.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_ROOT, 19, "/ramdisk/mod/rtc.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_ROOT, 20, "/ramdisk/mod/uart.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_ROOT, 19, "/ramdisk/mod/arp.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_ROOT, 20, "/ramdisk/mod/ipv4.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

}

void load_modules_testing(unsigned int id)
{

    /* Experimental */
    call_open(FUDGE_IN, FUDGE_ROOT, 19, "/ramdisk/mod/ata.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    /* Experimental */
    call_open(FUDGE_IN, FUDGE_ROOT, 20, "/ramdisk/mod/ext2.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    /* Does not work if device is non-existing */
    call_open(FUDGE_IN, FUDGE_ROOT, 23, "/ramdisk/mod/rtl8139.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    /* Does not work if device is non-existing */
    call_open(FUDGE_IN, FUDGE_ROOT, 19, "/ramdisk/mod/bga.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

}

void load_modules()
{

    call_open(3, FUDGE_ROOT, 17, "/ramdisk/bin/load");

    load_modules_core(3);
    load_modules_extra(3);

    call_close(3);

}

void mount_filesystems()
{

    call_open(3, FUDGE_ROOT, 8, "/system/");
    call_open(4, FUDGE_ROOT, 22, "/ramdisk/mod/system.ko");
    call_mount(3, 3, 4);
    call_close(4);
    call_close(3);

    call_open(3, FUDGE_ROOT, 12, "/system/tty/");
    call_open(4, FUDGE_ROOT, 19, "/ramdisk/mod/tty.ko");
    call_mount(4, 3, 4);
    call_close(4);
    call_close(3);

    call_open(3, FUDGE_ROOT, 15, "/system/nodefs/");
    call_open(4, FUDGE_ROOT, 22, "/ramdisk/mod/nodefs.ko");
    call_mount(5, 3, 4);
    call_close(4);
    call_close(3);

    call_open(3, FUDGE_ROOT, 14, "/system/video/");
    call_open(4, FUDGE_ROOT, 21, "/ramdisk/mod/video.ko");
    call_mount(6, 3, 4);
    call_close(4);
    call_close(3);

}

void start_shell()
{

    call_open(3, FUDGE_ROOT, 15, "/system/tty/cwd");
    call_write(3, 0, 14, "/ramdisk/home/");
    call_close(3);

    call_open(FUDGE_IN, FUDGE_ROOT, 25, "/system/nodefs/ps2_buffer");
    call_open(FUDGE_OUT, FUDGE_ROOT, 20, "/system/video/0/data");

    call_open(3, FUDGE_ROOT, 17, "/ramdisk/bin/motd");
    call_spawn(3);
    call_close(3);

    call_open(3, FUDGE_ROOT, 18, "/ramdisk/bin/shell");
    call_execute(3);
    call_close(3);

}

void main()
{

    load_modules();
    mount_filesystems();
    start_shell();

}

