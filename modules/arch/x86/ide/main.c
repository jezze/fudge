#include <fudge/module.h>
#include <base/base.h>
#include "ide.h"

enum ide_register
{

    IDE_REGISTER_BUS1DATA1              = 0x0168,
    IDE_REGISTER_BUS0DATA1              = 0x0170,
    IDE_REGISTER_BUS1DATA0              = 0x01E8,
    IDE_REGISTER_BUS0DATA0              = 0x01F0,
    IDE_REGISTER_BUS1CONTROL1           = 0x0366,
    IDE_REGISTER_BUS0CONTROL1           = 0x0376,
    IDE_REGISTER_BUS1CONTROL0           = 0x03E6,
    IDE_REGISTER_BUS0CONTROL0           = 0x03F6

};

static struct ide_bus primary;
static struct ide_bus secondary;

void init()
{

    unsigned int i;

    ide_init_bus(&primary, IDE_REGISTER_BUS0CONTROL0, IDE_REGISTER_BUS0DATA0);
    base_register_bus(&primary.base);

    for (i = 0; i < primary.devices.count; i++)
        base_register_device(&primary.devices.item[i].base);

    ide_init_bus(&secondary, IDE_REGISTER_BUS0CONTROL1, IDE_REGISTER_BUS0DATA1);
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

