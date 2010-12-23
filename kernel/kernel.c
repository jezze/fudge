#include <lib/cbuffer.h>
#include <kernel/dev.h>
#include <kernel/kernel.h>
#include <kernel/shell.h>
#include <kernel/tty.h>
#include <arch/x86/kernel/io.h>
#include <arch/x86/modules/pit/pit.h>
#include <arch/x86/modules/rtc/rtc.h>
#include <arch/x86/modules/ata/ata.h>
#include <arch/x86/modules/kbd/kbd.h>
#include <arch/x86/modules/vga/vga.h>

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

void kernel_init()
{

    kernel_init_devices();

    tty_init();
    shell_init();

    for (;;);

}

