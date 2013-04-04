#include <fudge.h>

void load_modules_core(unsigned int id)
{

    call_open(FUDGE_IN, FUDGE_ROOT, 13, "mod/system.ko");
    call_spawn(id);
    call_close(FUDGE_IN);
    call_open(FUDGE_IN, FUDGE_ROOT, 11, "mod/temp.ko");
    call_spawn(id);
    call_close(FUDGE_IN);
    call_open(FUDGE_IN, FUDGE_ROOT, 11, "mod/base.ko");
    call_spawn(id);
    call_close(FUDGE_IN);
    call_open(FUDGE_IN, FUDGE_ROOT, 15, "mod/terminal.ko");
    call_spawn(id);
    call_close(FUDGE_IN);
    call_open(FUDGE_IN, FUDGE_ROOT, 12, "mod/video.ko");
    call_spawn(id);
    call_close(FUDGE_IN);
    call_open(FUDGE_IN, FUDGE_ROOT, 10, "mod/net.ko");
    call_spawn(id);
    call_close(FUDGE_IN);
    call_open(FUDGE_IN, FUDGE_ROOT, 12, "mod/block.ko");
    call_spawn(id);
    call_close(FUDGE_IN);
    call_open(FUDGE_IN, FUDGE_ROOT, 10, "mod/kbd.ko");
    call_spawn(id);
    call_close(FUDGE_IN);
    call_open(FUDGE_IN, FUDGE_ROOT, 12, "mod/mouse.ko");
    call_spawn(id);
    call_close(FUDGE_IN);
    call_open(FUDGE_IN, FUDGE_ROOT, 9, "mod/io.ko");
    call_spawn(id);
    call_close(FUDGE_IN);
    call_open(FUDGE_IN, FUDGE_ROOT, 12, "mod/cpuid.ko");
    call_spawn(id);
    call_close(FUDGE_IN);
    call_open(FUDGE_IN, FUDGE_ROOT, 10, "mod/msr.ko");
    call_spawn(id);
    call_close(FUDGE_IN);
    call_open(FUDGE_IN, FUDGE_ROOT, 10, "mod/pic.ko");
    call_spawn(id);
    call_close(FUDGE_IN);
    call_open(FUDGE_IN, FUDGE_ROOT, 10, "mod/vga.ko");
    call_spawn(id);
    call_close(FUDGE_IN);
    call_open(FUDGE_IN, FUDGE_ROOT, 10, "mod/ps2.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

}

void load_modules_extra(unsigned int id)
{

/*
    call_open(FUDGE_IN, FUDGE_ROOT, 11, "mod/acpi.ko");
    call_spawn(id);
    call_close(FUDGE_IN);
*/

    call_open(FUDGE_IN, FUDGE_ROOT, 10, "mod/fpu.ko");
    call_spawn(id);
    call_close(FUDGE_IN);
    call_open(FUDGE_IN, FUDGE_ROOT, 10, "mod/pci.ko");
    call_spawn(id);
    call_close(FUDGE_IN);
    call_open(FUDGE_IN, FUDGE_ROOT, 10, "mod/pit.ko");
    call_spawn(id);
    call_close(FUDGE_IN);
    call_open(FUDGE_IN, FUDGE_ROOT, 10, "mod/rtc.ko");
    call_spawn(id);
    call_close(FUDGE_IN);
    call_open(FUDGE_IN, FUDGE_ROOT, 10, "mod/arp.ko");
    call_spawn(id);
    call_close(FUDGE_IN);
    call_open(FUDGE_IN, FUDGE_ROOT, 11, "mod/ipv4.ko");
    call_spawn(id);
    call_close(FUDGE_IN);
    call_open(FUDGE_IN, FUDGE_ROOT, 11, "mod/ipv6.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

}

void load_modules_testing(unsigned int id)
{

    /* Experimental */
    call_open(FUDGE_IN, FUDGE_ROOT, 10, "mod/ide.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    /* Experimental */
    call_open(FUDGE_IN, FUDGE_ROOT, 10, "mod/ata.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    /* Experimental */
    call_open(FUDGE_IN, FUDGE_ROOT, 12, "mod/atapi.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    /* Experimental */
    call_open(FUDGE_IN, FUDGE_ROOT, 11, "mod/ext2.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    /* Does not work if device is non-existing */
    call_open(FUDGE_IN, FUDGE_ROOT, 11, "mod/uart.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    /* Does not work if device is non-existing */
    call_open(FUDGE_IN, FUDGE_ROOT, 14, "mod/rtl8139.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

    /* Does not work if device is non-existing */
    call_open(FUDGE_IN, FUDGE_ROOT, 10, "mod/bga.ko");
    call_spawn(id);
    call_close(FUDGE_IN);

}

void load_modules()
{

    call_open(3, FUDGE_ROOT, 8, "bin/load");
    load_modules_core(3);
    load_modules_extra(3);
    call_close(3);

}

void mount_filesystems()
{

    call_open(3, FUDGE_ROOT, 7, "system/");
    call_open(4, FUDGE_ROOT, 13, "mod/system.ko");
    call_mount(2, 3, 4);
    call_close(4);
    call_close(3);
    call_open(3, FUDGE_ROOT, 5, "temp/");
    call_open(4, FUDGE_ROOT, 11, "mod/temp.ko");
    call_mount(3, 3, 4);
    call_close(4);
    call_close(3);

}

void start_shell()
{

    call_open(FUDGE_IN, FUDGE_ROOT, 17, "system/ps2_buffer");
    call_open(FUDGE_OUT, FUDGE_ROOT, 24, "system/terminal/vga/data");
    call_open(3, FUDGE_ROOT, 8, "bin/motd");
    call_spawn(3);
    call_close(3);
    call_open(3, FUDGE_ROOT, 9, "bin/shell");
    call_execute(3);
    call_close(3);

}

void main()
{

    call_open(FUDGE_CWD, FUDGE_ROOT, 5, "home/");
    load_modules();
    mount_filesystems();
    start_shell();

}

