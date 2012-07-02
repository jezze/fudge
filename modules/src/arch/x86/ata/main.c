#include <modules.h>
#include <arch/x86/ata/ata.h>

static struct ata_bus primary;
static struct ata_bus secondary;

void init()
{

    ata_bus_init(&primary, ATA_PRIMARY_MASTER_CONTROL, ATA_PRIMARY_MASTER_DATA);
    ata_bus_init(&secondary, ATA_PRIMARY_SLAVE_CONTROL, ATA_PRIMARY_SLAVE_DATA);

    modules_register_bus(&primary.base);
    modules_register_bus(&secondary.base);

}

void destroy()
{

    modules_unregister_bus(&primary.base);
    modules_unregister_bus(&secondary.base);

}

