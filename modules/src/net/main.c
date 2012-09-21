#include <vfs.h>
#include <base/base.h>
#include <net/net.h>

static struct net_filesystem filesystem;

void net_handle_read(struct net_interface *interface)
{

}

void net_register_interface(struct net_interface *interface, struct base_driver *driver)
{

    interface->driver = driver;

    filesystem.interfaces[filesystem.interfacesCount] = interface;
    filesystem.interfacesCount++;

}

void net_register_protocol(unsigned short index, struct net_protocol *protocol)
{

    filesystem.protocols[index] = protocol;

}

void net_unregister_interface(struct net_interface *interface)
{

}

void net_unregister_protocol(unsigned short index)
{

    filesystem.protocols[index] = 0;

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

