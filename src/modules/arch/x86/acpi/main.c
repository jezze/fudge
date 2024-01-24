#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/idt.h>
#include <kernel/x86/tss.h>
#include <kernel/x86/arch.h>
#include "acpi.h"

volatile unsigned int *address = (volatile unsigned int *)0x40E;

static struct acpi_rsdp *findrsdp(void)
{

    char *signature = "RSD PTR ";
    unsigned int rsdp;
    unsigned int ebda;

    for (rsdp = 0x000E0000; rsdp < 0x00100000; rsdp += 0x10)
    {

        if (buffer_match((void *)rsdp, signature, 8))
            return (struct acpi_rsdp *)rsdp;

    }

    ebda = *address;
    ebda = ebda * 0x10 & 0x000FFFFF;

    for (rsdp = ebda; rsdp < ebda + 0x400; rsdp += 0x10)
    {

        if (buffer_match((void *)rsdp, signature, 8))
            return (struct acpi_rsdp *)rsdp;

    }

    return 0;

}

struct acpi_sdth *acpi_findheader(char *name)
{

    struct acpi_rsdp *rsdp = findrsdp();
    struct acpi_sdth *sdth = (struct acpi_sdth *)rsdp->rsdt;
    struct acpi_sdth **entries = (struct acpi_sdth **)(rsdp->rsdt + sizeof (struct acpi_sdth));
    unsigned int total = (sdth->length - sizeof (struct acpi_sdth)) / 4;
    unsigned int i;

    for (i = 0; i < total; i++)
    {

        if (buffer_match(entries[i]->signature, name, 4))
            return entries[i];

    }

    return 0;

}

void module_init(void)
{

    struct acpi_rsdp *rsdp = findrsdp();

    if (!rsdp)
        return;

    arch_mapuncached(6, rsdp->rsdt, rsdp->rsdt, 0x00100000);

}

