#include <lib/memory.h>
#include <kernel/log.h>
#include <kernel/arch/x86/acpi.h>

static void *find_table(struct acpi_sdt *rsdt)
{

}

static void *get_rsdp()
{

    void *rsdp;

    for (rsdp = (void *)0x000E0000; rsdp < (void *)0x000FFFFF; rsdp += 0x10)
    {

        if (!memory_compare(rsdp, "RSD PTR ", 8))
            return rsdp;

    }

    return 0;

}

void acpi_init()
{

    struct acpi_rsdp *rsdp = get_rsdp();

    if (!rsdp)
        return;

    log_write("[acpi] RSDP Address: 0x%x\n", rsdp);
    log_write("[acpi] RSDP Revision: %d.0\n", rsdp->revision + 1);
    log_write("[acpi] RSDP OEM: %s\n", rsdp->oem);

    log_write("[acpi] RSDT Address: 0x%x\n", rsdp->rsdt);

    struct acpi_sdt *rsdt = (struct acpi_sdt *)rsdp->rsdt;

    log_write("[acpi] RSDT OEM: %s\n", rsdt->oem);

}

