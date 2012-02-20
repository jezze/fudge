#include <lib/string.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/nodefs/nodefs.h>
#include <modules/pit/pit.h>

static struct pit_device device;
static struct vfs_node jiffies;

static unsigned int jiffies_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    string_write_format(buffer, "%d", device.jiffies);

    return string_length(buffer);

}

void init()
{

    pit_device_init(&device);
    modules_register_device(&device.base);

    struct nodefs_driver *nodefs = (struct nodefs_driver *)modules_get_driver(NODEFS_DRIVER_TYPE);

    if (!nodefs)
        return;

    vfs_node_init(&jiffies, "module/pit/jiffies", 0, 0, jiffies_read, 0);

    nodefs->register_node(nodefs, &jiffies);

}

void destroy()
{

    modules_unregister_device(&device.base);

}

