#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/arch.h>
#include "acpi.h"

static struct acpi_rsdp *findrsdp(void)
{

    unsigned int rsdp;
    unsigned int ebda;
    char *signature = "RSD PTR ";

    for (rsdp = 0x000E0000; rsdp < 0x00100000; rsdp += 0x10)
    {

        if (memory_match((void *)rsdp, signature, 8))
            return (struct acpi_rsdp *)rsdp;

    }

    ebda = *((unsigned int *)0x40E);
    ebda = ebda * 0x10 & 0x000FFFFF;

    for (rsdp = ebda; rsdp < ebda + 0x400; rsdp += 0x10)
    {

        if (memory_match((void *)rsdp, signature, 8))
            return (struct acpi_rsdp *)rsdp;

    }

    return 0;

}

struct acpi_sdth *acpi_findheader(char *name)
{

    unsigned int i;
    struct acpi_rsdp *rsdp = findrsdp();
    unsigned int entries = (rsdp->rsdt->base.length - sizeof (struct acpi_sdth)) / 4;

    for (i = 0; i < entries; i++)
    {

        if (memory_match(rsdp->rsdt->entries[i]->signature, name, 4))
            return rsdp->rsdt->entries[i];

    }

    return 0;

}

void module_init(void)
{

    struct acpi_rsdp *rsdp = findrsdp();

    if (!rsdp)
        return;

    arch_setmap(6, (unsigned int)rsdp->rsdt, (unsigned int)rsdp->rsdt, 0x00100000);

}

