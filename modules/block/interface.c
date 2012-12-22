#include <fudge/memory.h>
#include <kernel/vfs.h>
#include <base/base.h>
#include "block.h"

void block_init_interface(struct block_interface *interface, struct base_driver *driver)
{

    memory_clear(interface, sizeof (struct block_interface));

    interface->driver = driver;

}

