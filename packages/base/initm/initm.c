#include <fudge.h>

void load_modules_core(unsigned int id)
{

    call_open(FUDGE_IN, FUDGE_CWD, 9, "system.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_CWD, 9, "nodefs.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_CWD, 8, "video.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_CWD, 6, "net.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_CWD, 8, "block.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_CWD, 5, "io.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_CWD, 8, "cpuid.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_CWD, 6, "msr.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_CWD, 6, "pic.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_CWD, 6, "vga.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_CWD, 6, "ps2.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

}

void load_modules_extra(unsigned int id)
{

/*
    call_open(FUDGE_IN, FUDGE_CWD, 7, "acpi.ko");
    call_spawn(id);
    call_close(FUDGE_IN);
*/

    call_open(FUDGE_IN, FUDGE_CWD, 6, "fpu.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_CWD, 6, "pci.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_CWD, 6, "pit.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_CWD, 6, "rtc.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_CWD, 6, "arp.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    call_open(FUDGE_IN, FUDGE_CWD, 7, "ipv4.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

}

void load_modules_testing(unsigned int id)
{

    /* Experimental */
    call_open(FUDGE_IN, FUDGE_CWD, 6, "ata.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    /* Experimental */
    call_open(FUDGE_IN, FUDGE_CWD, 7, "ext2.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    /* Does noot work if device is non-existing */
    call_open(FUDGE_IN, FUDGE_CWD, 7, "uart.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    /* Does not work if device is non-existing */
    call_open(FUDGE_IN, FUDGE_CWD, 10, "rtl8139.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    /* Does not work if device is non-existing */
    call_open(FUDGE_IN, FUDGE_CWD, 6, "bga.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

}

void load_modules()
{

    call_open(3, FUDGE_ROOT, 16, "ramdisk/bin/load");

    load_modules_core(3);
    load_modules_extra(3);

    call_close(3);

}

void mount_filesystems()
{

    call_open(3, FUDGE_ROOT, 7, "system/");
    call_open(4, FUDGE_CWD, 9, "system.ko");
    call_mount(3, 3, 4);
    call_close(4);
    call_close(3);

    call_open(3, FUDGE_ROOT, 14, "system/nodefs/");
    call_open(4, FUDGE_CWD, 9, "nodefs.ko");
    call_mount(4, 3, 4);
    call_close(4);
    call_close(3);

    call_open(3, FUDGE_ROOT, 13, "system/video/");
    call_open(4, FUDGE_CWD, 8, "video.ko");
    call_mount(5, 3, 4);
    call_close(4);
    call_close(3);

}

void start_shell()
{

    call_open(FUDGE_IN, FUDGE_ROOT, 24, "system/nodefs/ps2_buffer");
    call_open(FUDGE_OUT, FUDGE_ROOT, 19, "system/video/0/data");

    call_open(3, FUDGE_ROOT, 16, "ramdisk/bin/motd");
    call_spawn(3);
    call_close(3);

    call_open(3, FUDGE_ROOT, 17, "ramdisk/bin/shell");
    call_execute(3);
    call_close(3);

}

void main()
{

    call_open(FUDGE_CWD, FUDGE_ROOT, 12, "ramdisk/mod/");

    load_modules();
    mount_filesystems();

    call_open(FUDGE_CWD, FUDGE_ROOT, 0, 0);

    start_shell();

}

