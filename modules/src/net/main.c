#include <modules.h>
#include <net/net.h>

static struct net_driver driver;
static struct net_filesystem filesystem;

void net_register_interface(struct net_interface *interface, unsigned int (*read)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer))
{

    driver.register_interface(&driver, interface, read, write);

}

void net_unregister_interface(struct net_interface *interface)
{

    driver.unregister_interface(&driver, interface);

}

struct modules_filesystem *get_filesystem()
{

    return &filesystem.base;

}

void init()
{

    net_driver_init(&driver);
    modules_register_driver(&driver.base);

    net_filesystem_init(&filesystem);
    modules_register_filesystem(&filesystem.base);

}

void destroy()
{

    modules_unregister_filesystem(&filesystem.base);
    modules_unregister_driver(&driver.base);

}

