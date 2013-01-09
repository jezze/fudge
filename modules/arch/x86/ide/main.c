#include <fudge/module.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include <base/base.h>
#include "ide.h"

static struct ide_bus primary;
static struct ide_bus secondary;

void init()
{

    ide_init_bus(&primary, IDE_BUS0_COMMAND0, IDE_BUS0_DATA0);
    ide_init_bus(&secondary, IDE_BUS0_COMMAND1, IDE_BUS0_DATA1);
    base_register_bus(&primary.base);
    base_register_bus(&secondary.base);

}

void destroy()
{

    base_unregister_bus(&secondary.base);
    base_unregister_bus(&primary.base);

}

