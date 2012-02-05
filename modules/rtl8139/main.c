#include <lib/string.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/nodefs/nodefs.h>
#include <modules/rtl8139/rtl8139.h>

static struct rtl8139_driver driver;
static struct vfs_node mac;

static unsigned int mac_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    string_write_format(buffer, "%x:%x:%x:%x:%x:%x", driver.mac[0], driver.mac[1], driver.mac[2], driver.mac[3], driver.mac[4], driver.mac[5]);

    return string_length(buffer) + 1;

}

void init()
{

    rtl8139_driver_init(&driver);
    modules_register_driver(&driver.base);

    driver.base.start(&driver.base);

    struct nodefs_driver *nodefs = (struct nodefs_driver *)modules_get_driver(NODEFS_DRIVER_TYPE);

    if (!nodefs)
        return;

    vfs_node_init(&mac, "module/rtl8139/mac", 1, 0, 0, mac_read, 0);

    nodefs->register_node(nodefs, &mac);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

