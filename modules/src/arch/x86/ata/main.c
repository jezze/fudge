#include <modules.h>
#include <block/block.h>
#include <arch/x86/ata/ata.h>

static struct ata_bus primary;
static struct ata_bus secondary;
static struct ata_driver driver;

void init()
{

    ata_bus_init(&primary, ATA_PRIMARY_MASTER_CONTROL, ATA_PRIMARY_MASTER_DATA);
    ata_bus_init(&secondary, ATA_PRIMARY_SLAVE_CONTROL, ATA_PRIMARY_SLAVE_DATA);

    modules_register_bus(&primary.base);
    modules_register_bus(&secondary.base);

    ata_driver_init(&driver);
    modules_register_driver(&driver.base);

}

void destroy()
{

    modules_unregister_driver(&driver.base);
    modules_unregister_bus(&primary.base);
    modules_unregister_bus(&secondary.base);

}

