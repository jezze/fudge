#include <vfs.h>
#include <modules/modules.h>
#include <net/net.h>

static struct net_filesystem filesystem;

void net_register_interface(struct net_interface *interface, struct modules_driver *driver)
{

    interface->driver = driver;

    filesystem.interfaces[filesystem.interfacesCount] = interface;
    filesystem.interfacesCount++;

}

void net_register_protocol(struct net_protocol *protocol)
{

    filesystem.protocols[filesystem.protocolsCount] = protocol;
    filesystem.protocolsCount++;

}

void net_unregister_interface(struct net_interface *interface)
{

}

void net_unregister_protocol(struct net_protocol *protocol)
{

}

struct vfs_interface *get_filesystem()
{

    return &filesystem.base;

}

void init()
{

    net_filesystem_init(&filesystem);

}

void destroy()
{

}

