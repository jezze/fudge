#include <lib/memory.h>
#include <kernel/log.h>
#include <modules/acpi/acpi.h>

static void *write_table(struct acpi_sdth *entry)
{

    char *sign = memory_copy("XXXX\0", entry->signature, 4);

    log_write("[acpi] Table: %s\n", sign);

    return 0;

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

void init()
{

    struct acpi_rsdp *rsdp = get_rsdp();

    if (!rsdp)
        return;

    log_write("[acpi] RSDP Address: 0x%x\n", rsdp);
    log_write("[acpi] RSDP Revision: %d.0\n", rsdp->revision + 1);
    log_write("[acpi] RSDP OEM: %s\n", rsdp->oem);

    unsigned int i;

    for (i = 0; i < rsdp->length; i++)
        write_table((struct acpi_sdth *)(rsdp->rsdt + i * 4));

}

void destroy()
{

}

