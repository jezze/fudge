#include <memory.h>
#include <modules.h>
#include <mmu.h>
#include <arch/x86/acpi/acpi.h>

static struct acpi_sdth *find_header(struct acpi_driver *self, char *name)
{

    unsigned int i;
    unsigned int entries = (self->rsdp->rsdt->base.length - sizeof (struct acpi_sdth)) / 4;

    for (i = 0; i < entries; i++)
    {

/*
        if (memory_compare(self->rsdp->rsdt->entries[i]->signature, name, 4))
            return self->rsdp->rsdt->entries[i];
*/

    }

    return 0;

}

static unsigned int find_rsdp()
{

    unsigned int rsdp;
    unsigned int ebda;
    char *signature = "RSD PTR ";

    for (rsdp = 0x000E0000; rsdp < 0x00100000; rsdp += 0x10)
    {

        if (memory_compare((void *)rsdp, signature, 8))
            return rsdp;

    }

    ebda = *((unsigned int *)0x40E);
    ebda = ebda * 0x10 & 0x000FFFFF;

    for (rsdp = ebda; rsdp < ebda + 0x400; rsdp += 0x10)
    {

        if (memory_compare((void *)rsdp, signature, 8))
            return rsdp;

    }

    return 0;

}

static void start(struct modules_driver *self)
{

    struct acpi_driver *driver = (struct acpi_driver *)self;

    driver->rsdp = (void *)find_rsdp();

    if (!driver->rsdp)
        return;

    mmu_map_kernel_memory(1, (unsigned int)driver->rsdp->rsdt, (unsigned int)driver->rsdp->rsdt, 0x00100000);
    mmu_reload_memory();

}

void acpi_driver_init(struct acpi_driver *driver)
{

    memory_clear(driver, sizeof (struct acpi_driver));

    modules_driver_init(&driver->base, ACPI_DRIVER_TYPE, "acpi", start, 0, 0);

    driver->find_header = find_header;

}

