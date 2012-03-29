#include <lib/string.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/nodefs/nodefs.h>
#include <modules/pit/pit.h>

static struct pit_device device;
static struct pit_driver driver;
static struct nodefs_node jiffies;

static unsigned int jiffies_read(struct nodefs_node *self, unsigned int count, void *buffer)
{

    string_write(buffer, "%d", device.jiffies);

    return string_length(buffer);

}

void init()
{

    pit_device_init(&device, 0x00);
    modules_register_device(&device.base);

    pit_driver_init(&driver);
    modules_register_driver(&driver.base);

    struct nodefs_driver *nodefs = (struct nodefs_driver *)modules_get_driver(NODEFS_DRIVER_TYPE);

    if (!nodefs)
        return;

    nodefs->register_node(nodefs, &jiffies, "pit/jiffies", &driver.base.module, jiffies_read, 0);

}

void destroy()
{

    modules_unregister_device(&device.base);

}

