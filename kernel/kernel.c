#include <lib/cbuffer.h>
#include <kernel/dev.h>
#include <kernel/initrd.h>
#include <kernel/kernel.h>
#include <kernel/shell.h>
#include <kernel/tty.h>
#include <kernel/vfs.h>
#include <arch/x86/kernel/io.h>
#include <arch/x86/modules/pit/pit.h>
#include <arch/x86/modules/rtc/rtc.h>
#include <arch/x86/modules/ata/ata.h>
#include <arch/x86/modules/kbd/kbd.h>
#include <arch/x86/modules/vga/vga.h>

unsigned int kernelInitrdAddress;

void kernel_set_initrd(unsigned int initrdAddress)
{

    kernelInitrdAddress = initrdAddress;

}

static void kernel_init_devices()
{

    dev_init();
    io_init();
    vga_init();
    pit_init();
    kbd_init();
    rtc_init();
    ata_init();

}

static void kernel_init_filesystem()
{

    vfs_init();
    initrd_init(kernelInitrdAddress);

}

void kernel_init()
{

    kernel_init_filesystem();
    kernel_init_devices();

    tty_init();
    shell_init();

    for (;;);

}

