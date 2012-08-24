#include <memory.h>
#include <vfs.h>
#include <modules/modules.h>
#include <nodefs/nodefs.h>
#include <arch/x86/pit/pit.h>

static struct pit_device device;
static struct pit_driver driver;
static struct nodefs_node jiffies;

static unsigned int jiffies_read(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    memory_copy(buffer, &device.jiffies, 4);

    return 4;

}

void init()
{

    pit_device_init(&device, PIT_IRQ);
    modules_register_device(&device.base);

    pit_driver_init(&driver);
    modules_register_driver(&driver.base);

    nodefs_register_node(&jiffies, "pit_jiffies", &driver.base.base, jiffies_read, 0);

}

void destroy()
{

    modules_unregister_device(&device.base);

}

