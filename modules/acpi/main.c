#include <lib/memory.h>
#include <kernel/log.h>
#include <modules/acpi/acpi.h>

static void write_sdt(struct acpi_sdth *sdt)
{

    //log_write("[acpi] Sdt: %s\n", sdt->signature);

}

static void *get_rsdp()
{

    void *rsdp;

    for (rsdp = (void *)0x000E0000; rsdp < (void *)0x00100000; rsdp += 0x10)
    {

        if (!memory_compare(rsdp, "RSD PTR ", 8))
            return rsdp;

    }

    return 0;

}

void init()
{

    struct acpi_rsdp *rsdp = get_rsdp();

    if (!rsdp)
        return;

    log_write("[acpi] RSDP Address: 0x%x\n", rsdp);
    log_write("[acpi] RSDP Checksum: 0x%x\n", rsdp->checksum);
    log_write("[acpi] RSDP Revision: %d.0\n", rsdp->revision + 1);
    log_write("[acpi] RSDP OEM: %s\n", rsdp->oem);
    log_write("[acpi] RSDT Address: 0x%x\n", rsdp->rsdt);

    write_sdt((struct acpi_sdth *)rsdp->rsdt);

}

void destroy()
{

}

