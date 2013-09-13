#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include <base/network.h>
#include "net.h"

void net_register_protocol(unsigned short index, struct net_protocol *protocol)
{

}

void net_unregister_protocol(unsigned short index)
{

}

void net_init_protocol(struct net_protocol *protocol, char *name, unsigned int (*read)(struct base_network *interface, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct base_network *interface, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(protocol, sizeof (struct net_protocol));

    protocol->name = name;
    protocol->read = read;
    protocol->write = write;

}

void init()
{

}

void destroy()
{

}

