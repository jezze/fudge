#include <memory.h>
#include <string.h>
#include <vfs.h>
#include <base/base.h>
#include <dev/dev.h>

void dev_init_interface(struct dev_interface *interface, struct base_driver *driver)
{

    memory_clear(interface, sizeof (struct dev_interface));

    interface->driver = driver;

}

