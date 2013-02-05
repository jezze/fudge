#include <fudge/module.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include <base/base.h>
#include "ide.h"

static struct ide_bus primary;
static struct ide_bus secondary;

void init()
{

    unsigned int i;

    ide_init_bus(&primary, IDE_BUS0_COMMAND0, IDE_BUS0_DATA0);
    base_register_bus(&primary.base);

    for (i = 0; i < primary.devices.count; i++)
        base_register_device(&primary.devices.item[i].base);

    ide_init_bus(&secondary, IDE_BUS0_COMMAND1, IDE_BUS0_DATA1);
    base_register_bus(&secondary.base);

    for (i = 0; i < secondary.devices.count; i++)
        base_register_device(&secondary.devices.item[i].base);

}

void destroy()
{

    unsigned int i;

    for (i = 0; i < primary.devices.count; i++)
        base_unregister_device(&primary.devices.item[i].base);

    base_unregister_bus(&primary.base);

    for (i = 0; i < secondary.devices.count; i++)
        base_unregister_device(&secondary.devices.item[i].base);

    base_unregister_bus(&secondary.base);

}

