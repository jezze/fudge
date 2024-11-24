#include <fudge.h>
#include <abi.h>

static void loaddriver(unsigned int ichannel, char *path)
{

    unsigned int target = fs_spawn(ichannel, "initrd:bin/elfload");

    if (target)
    {

        channel_send(ichannel, target, EVENT_MAIN);
        channel_send_fmt1(ichannel, target, EVENT_PATH, "%s\\0", path);
        channel_send(ichannel, target, EVENT_END);
        channel_wait(ichannel, target, EVENT_DONE);

    }

}

static unsigned int spawnenv(unsigned int ichannel)
{

    unsigned int target = fs_spawn(ichannel, "initrd:bin/env");

    if (target)
    {

        channel_send_fmt1(ichannel, target, EVENT_OPTION, "env\\0%u\\0pwd\\0initrd:\\0", &target);
        channel_send(ichannel, target, EVENT_MAIN);

    }

    return target;

}

static unsigned int spawnshell(unsigned int ichannel, unsigned int env)
{

    unsigned int target = fs_spawn(ichannel, "initrd:bin/shell");

    if (target)
    {

        channel_send_fmt1(ichannel, target, EVENT_OPTION, "env\\0%u\\0pwd\\0initrd:\\0", &env);
        channel_send(ichannel, target, EVENT_MAIN);

    }

    return target;

}

static unsigned int spawnwm(unsigned int ichannel, unsigned int env)
{

    unsigned int target = fs_spawn(ichannel, "initrd:bin/wm");

    if (target)
    {

        channel_send_fmt1(ichannel, target, EVENT_OPTION, "env\\0%u\\0pwd\\0initrd:\\0", &env);
        channel_send(ichannel, target, EVENT_MAIN);

    }

    return target;

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int env;

    loaddriver(1, "initrd:kernel/base.ko");
    loaddriver(1, "initrd:kernel/log.ko");
    loaddriver(1, "initrd:kernel/block.ko");
    loaddriver(1, "initrd:kernel/clock.ko");
    loaddriver(1, "initrd:kernel/console.ko");
    loaddriver(1, "initrd:kernel/keyboard.ko");
    loaddriver(1, "initrd:kernel/mouse.ko");
    loaddriver(1, "initrd:kernel/timer.ko");
    loaddriver(1, "initrd:kernel/audio.ko");
    loaddriver(1, "initrd:kernel/video.ko");
    loaddriver(1, "initrd:kernel/ethernet.ko");
    loaddriver(1, "initrd:kernel/info.ko");
    loaddriver(1, "initrd:kernel/io.ko");
    loaddriver(1, "initrd:kernel/cpuid.ko");
    loaddriver(1, "initrd:kernel/msr.ko");
    loaddriver(1, "initrd:kernel/pat.ko");
    loaddriver(1, "initrd:kernel/acpi.ko");
    loaddriver(1, "initrd:kernel/pic.ko");
    loaddriver(1, "initrd:kernel/apic.ko");
    loaddriver(1, "initrd:kernel/platform.ko");
    loaddriver(1, "initrd:kernel/pci.ko");
    loaddriver(1, "initrd:kernel/pit.ko");
    loaddriver(1, "initrd:kernel/rtc.ko");
    loaddriver(1, "initrd:kernel/ps2.ko");
    loaddriver(1, "initrd:kernel/ps2-keyboard.ko");
    loaddriver(1, "initrd:kernel/ps2-mouse.ko");
    loaddriver(1, "initrd:kernel/bga.ko");
    loaddriver(1, "initrd:kernel/uart.ko");
    loaddriver(1, "initrd:kernel/vga.ko");
    loaddriver(1, "initrd:kernel/rtl8139.ko");
    loaddriver(1, "initrd:kernel/ide.ko");
    loaddriver(1, "initrd:kernel/ahci.ko");
    loaddriver(1, "initrd:kernel/nvme.ko");
    loaddriver(1, "initrd:kernel/ata.ko");
    loaddriver(1, "initrd:kernel/virtio-network.ko");

    /*
    loaddriver("initrd:kernel/smp.ko");
    */

    env = spawnenv(1);

    spawnshell(1, env);
    spawnwm(1, env);

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}

