#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int channel;

    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/base.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/system.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/log.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/block.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/clock.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/console.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/keyboard.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/mouse.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/timer.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/audio.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/video.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/ethernet.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/info.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);

    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/io.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/cpuid.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/msr.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/pat.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/acpi.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/pic.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/apic.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/platform.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/pci.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/pit.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/rtc.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/ps2.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/ps2-keyboard.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/ps2-mouse.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/bga.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/uart.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/vga.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/rtl8139.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/ide.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/ahci.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/nvme.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/ata.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);
    channel = fsp_spawn("initrd:bin/elfload");
    channel_send_fmt0(channel, EVENT_PATH, "initrd:kernel/virtio-network.ko\\0");
    channel_send(channel, EVENT_END);
    channel_wait(EVENT_TERMRESPONSE);

    channel = fsp_spawn("initrd:bin/shell");
    channel_send_fmt0(channel, EVENT_OPTION, "pwd\\0initrd:\\0");
    channel_send_fmt0(channel, EVENT_OPTION, "input\\0system:console/if.0/event\\0");
    channel_send_fmt0(channel, EVENT_OPTION, "tty-service\\0100\\0");
    channel_send(channel, EVENT_MAIN);

    channel = fsp_spawn("initrd:bin/wm");
    channel_send_fmt0(channel, EVENT_OPTION, "pwd\\0initrd:\\0");
    channel_send_fmt0(channel, EVENT_OPTION, "listen\\012345\\0");
    channel_send(channel, EVENT_MAIN);

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}

