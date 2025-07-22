#include <fudge.h>
#include <abi.h>

static void loadmodules(unsigned int ichannel, unsigned int count, char **paths)
{

    unsigned int target = fs_spawn(ichannel, "initrd:bin/elfload");

    if (target)
    {

        unsigned int i;

        channel_send(ichannel, target, EVENT_MAIN);

        for (i = 0; i < count; i++)
            channel_send_fmt1(ichannel, target, EVENT_PATH, "%s\\0", paths[i]);

        channel_send(ichannel, target, EVENT_END);
        channel_wait(ichannel, target, EVENT_DONE);

    }

}

static unsigned int spawnshell(unsigned int ichannel)
{

    unsigned int target = fs_spawn(ichannel, "initrd:bin/shell");

    if (target)
    {

        channel_send_fmt0(ichannel, target, EVENT_OPTION, "pwd=initrd:\n");
        channel_send(ichannel, target, EVENT_MAIN);

    }

    return target;

}

static unsigned int spawnwm(unsigned int ichannel)
{

    unsigned int target = fs_spawn(ichannel, "initrd:bin/wm");

    if (target)
    {

        channel_send_fmt0(ichannel, target, EVENT_OPTION, "pwd=initrd:\n");
        channel_send(ichannel, target, EVENT_MAIN);

    }

    return target;

}

static char *modules0[18] = {
    "initrd:kernel/base.ko",
    "initrd:kernel/log.ko",
    "initrd:kernel/block.ko",
    "initrd:kernel/clock.ko",
    "initrd:kernel/console.ko",
    "initrd:kernel/keyboard.ko",
    "initrd:kernel/mouse.ko",
    "initrd:kernel/timer.ko",
    "initrd:kernel/audio.ko",
    "initrd:kernel/video.ko",
    "initrd:kernel/ethernet.ko",
    "initrd:kernel/info.ko",
    "initrd:kernel/io.ko",
    "initrd:kernel/cpuid.ko",
    "initrd:kernel/msr.ko",
    "initrd:kernel/pat.ko",
    "initrd:kernel/acpi.ko",
    "initrd:kernel/pic.ko"
};

static char *modules1[1] = {
    "initrd:kernel/apic.ko"
};

static char *modules2[16] = {
    "initrd:kernel/platform.ko",
    "initrd:kernel/pci.ko",
    "initrd:kernel/pit.ko",
    "initrd:kernel/rtc.ko",
    "initrd:kernel/ps2.ko",
    "initrd:kernel/ps2-keyboard.ko",
    "initrd:kernel/ps2-mouse.ko",
    "initrd:kernel/bga.ko",
    "initrd:kernel/uart.ko",
    "initrd:kernel/vga.ko",
    "initrd:kernel/rtl8139.ko",
    "initrd:kernel/ide.ko",
    "initrd:kernel/ahci.ko",
    "initrd:kernel/nvme.ko",
    "initrd:kernel/ata.ko",
    "initrd:kernel/virtio-network.ko"
};

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    /* FIXME: Because of an mmu bug, apic module needs to load seperately. It kills the process. */
    loadmodules(1, 18, modules0);
    loadmodules(1, 1, modules1);
    loadmodules(1, 16, modules2);
    spawnshell(1);
    spawnwm(1);

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}

