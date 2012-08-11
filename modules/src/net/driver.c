#include <memory.h>
#include <modules.h>
#include <net/net.h>

static void register_interface(struct net_driver *self, struct net_interface *interface, struct modules_base *module, unsigned int (*read)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer))
{

    interface->read = read;
    interface->write = write;

    self->interfaces[self->interfacesCount] = interface;
    self->interfacesCount++;

}

static void register_protocol(struct net_driver *self, struct net_protocol *protocol, char *name)
{

    protocol->name = name;

    self->protocols[self->protocolsCount] = protocol;
    self->protocolsCount++;

}

static void unregister_interface(struct net_driver *self, struct net_interface *interface)
{

    self->interfacesCount--;

}

static void unregister_protocol(struct net_driver *self, struct net_protocol *protocol)
{

    self->protocolsCount--;

}

void net_driver_init(struct net_driver *driver)
{

    memory_clear(driver, sizeof (struct net_driver));

    modules_driver_init(&driver->base, NET_DRIVER_TYPE, "net", 0, 0, 0);

    driver->register_interface = register_interface;
    driver->register_protocol = register_protocol;
    driver->unregister_interface = unregister_interface;
    driver->unregister_protocol = unregister_protocol;

}

