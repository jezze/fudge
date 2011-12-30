#include <kernel/modules.h>
#include <modules/ata/ata.h>

static struct ata_bus primary;
static struct ata_bus secondary;

void init()
{

    ata_bus_init(&primary, ATA_MASTER_PRIMARY_CONTROL, ATA_MASTER_PRIMARY_DATA, 0x0E);
    ata_bus_init(&secondary, ATA_MASTER_SECONDARY_CONTROL, ATA_MASTER_SECONDARY_DATA, 0x0F);

    modules_register_bus(&primary.base);
    modules_register_bus(&secondary.base);

}

void destroy()
{

    modules_unregister_bus(&primary.base);
    modules_unregister_bus(&secondary.base);

}

