#include <memory.h>
#include <string.h>
#include <vfs.h>
#include <base/base.h>
#include "net.h"

void net_init_interface(struct net_interface *interface, struct base_driver *driver, unsigned int (*send)(struct net_interface *self, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct net_interface));

    interface->driver = driver;
    interface->send = send;

}

