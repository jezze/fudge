#include <vfs.h>
#include <base/base.h>
#include <block/block.h>
#include <arch/x86/ata/ata.h>

static struct ata_bus primary;
static struct ata_bus secondary;
static struct ata_driver driver;

void init()
{

    ata_init_bus(&primary, ATA_PRIMARY_MASTER_CONTROL, ATA_PRIMARY_MASTER_DATA);
    ata_init_bus(&secondary, ATA_PRIMARY_SLAVE_CONTROL, ATA_PRIMARY_SLAVE_DATA);

    base_register_bus(&primary.base);
    base_register_bus(&secondary.base);

    ata_init_driver(&driver);
    base_register_driver(&driver.base);
    block_register_interface(&driver.interface, &driver.base);

}

void destroy()
{

    block_unregister_interface(&driver.interface);
    base_unregister_driver(&driver.base);
    base_unregister_bus(&primary.base);
    base_unregister_bus(&secondary.base);

}

