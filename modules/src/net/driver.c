#include <memory.h>
#include <modules.h>
#include <net/net.h>

static void register_interface(struct net_driver *self, struct net_interface *interface, unsigned int (*read)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer))
{

    interface->read = read;
    interface->write = write;

    self->interfaces[self->interfacesCount] = interface;
    self->interfacesCount++;

}

static void unregister_interface(struct net_driver *self, struct net_interface *interface)
{

    self->interfaces[0] = 0;

}

void net_driver_init(struct net_driver *driver)
{

    memory_clear(driver, sizeof (struct net_driver));

    modules_driver_init(&driver->base, NET_DRIVER_TYPE, "net", 0, 0, 0);

    driver->register_interface = register_interface;
    driver->unregister_interface = unregister_interface;

}

