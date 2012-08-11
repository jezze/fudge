#include <modules.h>
#include <net/net.h>

static struct net_driver driver;

void net_register_interface(struct net_interface *interface, unsigned int (*read)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer))
{

    driver.register_interface(&driver, interface, read, write);

}

void net_unregister_interface(struct net_interface *interface)
{

    driver.unregister_interface(&driver, interface);

}

void init()
{

    net_driver_init(&driver);
    modules_register_driver(&driver.base);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

