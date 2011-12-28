#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/modules.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/vfs.h>
#include <modules/pci/pci.h>
#include <modules/rtl8139/rtl8139.h>

static struct rtl8139_driver driver;

void init()
{

    rtl8139_driver_init(&driver);
    modules_register_driver(&driver.base);
    modules_attach(&driver.base);

    driver.base.start(&driver.base);

}

void destroy()
{

}

