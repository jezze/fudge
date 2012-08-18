#include <modules.h>
#include <block/block.h>

static struct block_filesystem filesystem;

void block_register_driver(struct block_driver *driver, unsigned int (*read)(struct block_driver *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct block_driver *self, unsigned int offset, unsigned int count, void *buffer))
{

    filesystem.register_driver(&filesystem, driver, read, write);

}

void block_register_protocol(struct block_protocol *protocol, char *name)
{

    filesystem.register_protocol(&filesystem, protocol, name);

}

void block_unregister_driver(struct block_driver *driver)
{

    filesystem.unregister_driver(&filesystem, driver);

}

void block_unregister_protocol(struct block_protocol *protocol)
{

    filesystem.unregister_protocol(&filesystem, protocol);

}

struct modules_filesystem *get_filesystem()
{

    return &filesystem.base;

}

void init()
{

    block_filesystem_init(&filesystem);
    modules_register_filesystem(&filesystem.base);

}

void destroy()
{

    modules_unregister_filesystem(&filesystem.base);

}

