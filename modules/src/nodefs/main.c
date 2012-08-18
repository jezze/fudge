#include <modules.h>
#include <nodefs/nodefs.h>

static struct nodefs_driver driver;
static struct nodefs_filesystem filesystem;

void nodefs_register_node(struct nodefs_node *node, char *name, struct modules_base *module, unsigned int (*read)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer))
{

    driver.register_node(&driver, node, name, module, read, write);

}

void nodefs_unregister_node(struct nodefs_node *node)
{

    driver.unregister_node(&driver, node);

}

struct modules_filesystem *get_filesystem()
{

    return &filesystem.base;

}

void init()
{

    nodefs_driver_init(&driver);
    modules_register_driver(&driver.base);

    nodefs_filesystem_init(&filesystem, &driver);
    modules_register_filesystem(&filesystem.base);

}

void destroy()
{

    modules_unregister_filesystem(&filesystem.base);
    modules_unregister_driver(&driver.base);

}

