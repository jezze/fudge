#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <modules/pci/pci.h>
#include <modules/ata/ata.h>

static struct ata_bus primary;
static struct ata_bus secondary;
static unsigned int irq;

static void set_irq()
{

    irq = 1;

}

static void clear_irq()
{

    irq = 0;

}

static void wait_irq()
{

    while (!irq);

    clear_irq();

}

static void handle_irq_primary()
{

    set_irq();
    log_write("[ata] irq 0x0E\n");

}

static void handle_irq_secondary()
{

    set_irq();
    log_write("[ata] irq 0x0F\n");

}

void init()
{

    clear_irq();

    kernel_register_irq(0x0E, handle_irq_primary);
    kernel_register_irq(0x0F, handle_irq_secondary);

    ata_bus_init(&primary, ATA_MASTER_PRIMARY_CONTROL, ATA_MASTER_PRIMARY_DATA);
    modules_register_bus(&primary.base);

    ata_bus_init(&secondary, ATA_MASTER_SECONDARY_CONTROL, ATA_MASTER_SECONDARY_DATA);
    modules_register_bus(&secondary.base);

}

void destroy()
{

    kernel_unregister_irq(0x0E);
    kernel_unregister_irq(0x0F);

    modules_unregister_bus(&primary.base);
    modules_unregister_bus(&secondary.base);

}

