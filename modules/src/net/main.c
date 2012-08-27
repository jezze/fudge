#include <vfs.h>
#include <modules/modules.h>
#include <net/net.h>

static struct net_filesystem filesystem;

void net_register_interface(struct net_interface *interface, struct modules_driver *driver, unsigned int (*read)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct net_interface *self, unsigned int offset, unsigned int count, void *buffer))
{

    filesystem.register_interface(&filesystem, interface, driver, read, write);

}

void net_register_protocol(struct net_protocol *protocol, char *name)
{

    filesystem.register_protocol(&filesystem, protocol, name);

}

void net_unregister_interface(struct net_interface *interface)
{

    filesystem.unregister_interface(&filesystem, interface);

}

void net_unregister_protocol(struct net_protocol *protocol)
{

    filesystem.unregister_protocol(&filesystem, protocol);

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

