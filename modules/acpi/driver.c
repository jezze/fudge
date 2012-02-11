#include <lib/memory.h>
#include <kernel/log.h>
#include <kernel/modules.h>
#include <kernel/mmu.h>
#include <modules/acpi/acpi.h>

static struct acpi_sdth *get_table(void *rsdt, char *name)
{

    struct acpi_sdth *header = rsdt;
    struct acpi_sdth **headers = rsdt + sizeof (struct acpi_sdth);
    unsigned int entries = (header->length - sizeof (struct acpi_sdth)) / 4;

    unsigned int i;

    for (i = 0; i < entries; i++)
    {

        if (!memory_compare(headers[i]->signature, name, 4))
            return headers[i];

    }

    return 0;

}

static void *get_rsdp()
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

    struct acpi_madt *madt = (struct acpi_madt *)get_table((struct acpi_sdth *)driver->rsdp->rsdt, "APIC");

    log_write("[acpi] Madt signature: %c%c%c%c\n", madt->base.signature[0], madt->base.signature[1], madt->base.signature[2], madt->base.signature[3]);

}

void acpi_driver_init(struct acpi_driver *driver)
{

    modules_driver_init(&driver->base, ACPI_DRIVER_TYPE);

    driver->base.start = start;
    driver->rsdp = get_rsdp();

    mmu_memory_init(&driver->memory, (void *)driver->rsdp->rsdt, (void *)driver->rsdp->rsdt, 0x10000); 
    mmu_map_kernel_memory(&driver->memory);
    mmu_reload_memory();

}

