#include <lib/memory.h>
#include <kernel/modules.h>
#include <kernel/mmu.h>
#include <modules/acpi/acpi.h>

static struct mmu_memory memory;

static struct acpi_sdth *find_header(struct acpi_driver *self, char *name)
{

    unsigned int entries = (self->rsdp->rsdt->base.length - sizeof (struct acpi_sdth)) / 4;

    unsigned int i;

    for (i = 0; i < entries; i++)
    {

        if (!memory_compare(self->rsdp->rsdt->entries[i]->signature, name, 4))
            return self->rsdp->rsdt->entries[i];

    }

    return 0;

}

static void *find_rsdp()
{

    void *rsdp;
    char *signature = "RSD PTR ";

    for (rsdp = (void *)0x000E0000; rsdp < (void *)0x00100000; rsdp += 0x10)
    {

        if (!memory_compare(rsdp, signature, 8))
            return rsdp;

    }

    unsigned int ebda = *((unsigned int *)0x40E);
    ebda = ebda * 0x10 & 0x000FFFFF;

    for (rsdp = (void *)ebda; rsdp < (void *)(ebda + 0x400); rsdp += 0x10)
    {

        if (!memory_compare(rsdp, signature, 8))
            return rsdp;

    }

    return 0;

}

static void start(struct modules_driver *self)
{

    struct acpi_driver *driver = (struct acpi_driver *)self;

    driver->rsdp = find_rsdp();

    if (!driver->rsdp)
        return;

    mmu_memory_init(&memory, driver->rsdp->rsdt, driver->rsdp->rsdt, 0x10000); 
    mmu_map_kernel_memory(&memory);
    mmu_reload_memory();

}

void acpi_driver_init(struct acpi_driver *driver)
{

    memory_clear(driver, sizeof (struct acpi_driver));

    modules_driver_init(&driver->base, ACPI_DRIVER_TYPE, "acpi", start, 0, 0);

    driver->find_header = find_header;

}

