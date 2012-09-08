#include <vfs.h>
#include <modules/modules.h>
#include <block/block.h>

static struct block_filesystem filesystem;

void block_register_interface(struct block_interface *interface, struct modules_driver *driver, unsigned int (*read)(struct block_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct block_interface *self, unsigned int offset, unsigned int count, void *buffer))
{

    interface->driver = driver;
    interface->read = read;
    interface->write = write;

    filesystem.interfaces[filesystem.interfacesCount] = interface;
    filesystem.interfacesCount++;

}

void block_register_protocol(struct block_protocol *protocol, char *name)
{

    protocol->name = name;

    filesystem.protocols[filesystem.protocolsCount] = protocol;
    filesystem.protocolsCount++;

}

void block_unregister_interface(struct block_interface *interface)
{

}

void block_unregister_protocol(struct block_protocol *protocol)
{

}

struct vfs_interface *get_filesystem()
{

    return &filesystem.base;

}

void init()
{

    block_filesystem_init(&filesystem);

}

void destroy()
{

}

