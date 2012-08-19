#include <memory.h>
#include <modules.h>
#include <net/net.h>
#include <arch/x86/rtl8139/rtl8139.h>

static struct rtl8139_driver driver;

static unsigned int interface_read(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct rtl8139_driver *driver = (struct rtl8139_driver *)self->driver;

    return driver->read(driver, count, buffer);

}

static unsigned int interface_write(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct rtl8139_driver *driver = (struct rtl8139_driver *)self->driver;

    return driver->write(driver, count, buffer);

}

void init()
{

    rtl8139_driver_init(&driver);
    modules_register_driver(&driver.base);
    net_register_interface(&driver.interface, &driver.base, interface_read, interface_write);

}

void destroy()
{

    net_unregister_interface(&driver.interface);
    modules_unregister_driver(&driver.base);

}

