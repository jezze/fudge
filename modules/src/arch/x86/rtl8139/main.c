#include <memory.h>
#include <modules.h>
#include <net/net.h>
#include <arch/x86/rtl8139/rtl8139.h>

static struct rtl8139_driver driver;

static unsigned int read(struct net_driver *self, unsigned int offset, unsigned int count, void *buffer)
{

    return driver.read(&driver, count, buffer);

}

static unsigned int write(struct net_driver *self, unsigned int offset, unsigned int count, void *buffer)
{

    return driver.write(&driver, count, buffer);

}

void init()
{

    rtl8139_driver_init(&driver);
    net_register_driver(&driver.base, read, write);

}

void destroy()
{

    net_unregister_driver(&driver.base);

}

