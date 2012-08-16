#include <modules.h>
#include <block/block.h>

static struct block_driver driver;
static struct modules_filesystem filesystem;

void block_register_interface(struct block_interface *interface, struct modules_base *module, unsigned int (*read)(struct block_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct block_interface *self, unsigned int offset, unsigned int count, void *buffer))
{

    driver.register_interface(&driver, interface, module, read, write);

}

void block_register_protocol(struct block_protocol *protocol, char *name)
{

    driver.register_protocol(&driver, protocol, name);

}

void block_unregister_interface(struct block_interface *interface)
{

    driver.unregister_interface(&driver, interface);

}

void block_unregister_protocol(struct block_protocol *protocol)
{

    driver.unregister_protocol(&driver, protocol);

}

struct modules_filesystem *get_filesystem()
{

    return &filesystem;

}

void init()
{

    block_driver_init(&driver);
    modules_register_driver(&driver.base);

    block_filesystem_init(&filesystem, &driver.base);
    modules_register_filesystem(&filesystem);

}

void destroy()
{

    modules_unregister_filesystem(&filesystem);
    modules_unregister_driver(&driver.base);

}

