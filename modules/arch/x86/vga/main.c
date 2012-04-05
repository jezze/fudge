#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/nodefs/nodefs.h>
#include <modules/vga/vga.h>

static struct vga_driver driver;
static struct nodefs_node console;

void init()
{

    vga_driver_init(&driver);
    modules_register_driver(&driver.base);

    struct nodefs_driver *nodefsDriver = (struct nodefs_driver *)modules_get_driver(NODEFS_DRIVER_TYPE);

    if (!nodefsDriver)
        return;

    nodefsDriver->register_node(nodefsDriver, &console, "vga/console", &driver.base.base, 0, 0);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

