#include <fudge.h>
#include <abi.h>

static void loaddriver(char *path)
{

    unsigned int target = fs_spawn(0, "initrd:bin/elfload");

    if (target)
    {

        channel_send(0, target, EVENT_MAIN);
        channel_send_fmt1(0, target, EVENT_PATH, "%s\\0", path);
        channel_send(0, target, EVENT_END);
        channel_wait(0, target, EVENT_DONE);

    }

}

static unsigned int spawnenv(void)
{

    unsigned int target = fs_spawn(0, "initrd:bin/env");

    if (target)
    {

        channel_send_fmt1(0, target, EVENT_OPTION, "env\\0%u\\0pwd\\0initrd:\\0", &target);
        channel_send(0, target, EVENT_MAIN);

    }

    return target;

}

static unsigned int spawnshell(unsigned int env)
{

    unsigned int target = fs_spawn(0, "initrd:bin/shell");

    if (target)
    {

        channel_send_fmt1(0, target, EVENT_OPTION, "env\\0%u\\0pwd\\0initrd:\\0", &env);
        channel_send(0, target, EVENT_MAIN);

    }

    return target;

}

static unsigned int spawnwm(unsigned int env)
{

    unsigned int target = fs_spawn(0, "initrd:bin/wm");

    if (target)
    {

        channel_send_fmt1(0, target, EVENT_OPTION, "env\\0%u\\0pwd\\0initrd:\\0", &env);
        channel_send(0, target, EVENT_MAIN);

    }

    return target;

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int env;

    loaddriver("initrd:kernel/base.ko");
    loaddriver("initrd:kernel/log.ko");
    loaddriver("initrd:kernel/block.ko");
    loaddriver("initrd:kernel/clock.ko");
    loaddriver("initrd:kernel/console.ko");
    loaddriver("initrd:kernel/keyboard.ko");
    loaddriver("initrd:kernel/mouse.ko");
    loaddriver("initrd:kernel/timer.ko");
    loaddriver("initrd:kernel/audio.ko");
    loaddriver("initrd:kernel/video.ko");
    loaddriver("initrd:kernel/ethernet.ko");
    loaddriver("initrd:kernel/info.ko");
    loaddriver("initrd:kernel/io.ko");
    loaddriver("initrd:kernel/cpuid.ko");
    loaddriver("initrd:kernel/msr.ko");
    loaddriver("initrd:kernel/pat.ko");
    loaddriver("initrd:kernel/acpi.ko");
    loaddriver("initrd:kernel/pic.ko");
    loaddriver("initrd:kernel/apic.ko");
    loaddriver("initrd:kernel/platform.ko");
    loaddriver("initrd:kernel/pci.ko");
    loaddriver("initrd:kernel/pit.ko");
    loaddriver("initrd:kernel/rtc.ko");
    loaddriver("initrd:kernel/ps2.ko");
    loaddriver("initrd:kernel/ps2-keyboard.ko");
    loaddriver("initrd:kernel/ps2-mouse.ko");
    loaddriver("initrd:kernel/bga.ko");
    loaddriver("initrd:kernel/uart.ko");
    loaddriver("initrd:kernel/vga.ko");
    loaddriver("initrd:kernel/rtl8139.ko");
    loaddriver("initrd:kernel/ide.ko");
    loaddriver("initrd:kernel/ahci.ko");
    loaddriver("initrd:kernel/nvme.ko");
    loaddriver("initrd:kernel/ata.ko");
    loaddriver("initrd:kernel/virtio-network.ko");

    /*
    loaddriver("initrd:kernel/smp.ko");
    */

    env = spawnenv();

    spawnshell(env);
    spawnwm(env);

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}

