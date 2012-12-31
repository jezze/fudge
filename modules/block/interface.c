#include <fudge/memory.h>
#include <base/base.h>
#include "block.h"

void block_init_interface(struct block_interface *interface, struct base_driver *driver, unsigned int (*read)(struct block_interface *self, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct block_interface *self, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct block_interface));

    interface->driver = driver;
    interface->read = read;
    interface->write = write;

}

