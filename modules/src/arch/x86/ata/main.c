#include <vfs.h>
#include <base/base.h>
#include <block/block.h>
#include <arch/x86/ata/ata.h>

static struct ata_bus primary;
static struct ata_bus secondary;
static struct ata_driver driver;

void init()
{

    ata_init_bus(&primary, ATA_BUS0_COMMAND0, ATA_BUS0_DATA0);
    ata_init_bus(&secondary, ATA_BUS0_COMMAND1, ATA_BUS0_DATA1);

    base_register_bus(&primary.base);
    base_register_bus(&secondary.base);

    ata_init_driver(&driver);
    base_register_driver(&driver.base);
    block_register_interface(&driver.interface);

}

void destroy()
{

    block_unregister_interface(&driver.interface);
    base_unregister_driver(&driver.base);
    base_unregister_bus(&primary.base);
    base_unregister_bus(&secondary.base);

}

