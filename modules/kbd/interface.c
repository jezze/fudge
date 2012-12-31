#include <fudge/memory.h>
#include <base/base.h>
#include <system/system.h>
#include "kbd.h"

void kbd_init_interface(struct kbd_interface *interface, struct base_driver *driver)
{

    memory_clear(interface, sizeof (struct kbd_interface));

    interface->driver = driver;

}

