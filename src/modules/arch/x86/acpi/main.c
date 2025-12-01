#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/idt.h>
#include <kernel/x86/tss.h>
#include <kernel/x86/arch.h>
#include "acpi.h"

volatile unsigned int *pointer = (volatile unsigned int *)0x40E;

static unsigned int validate(void *address, unsigned int length)
{

    unsigned char *x = address;
    unsigned int sum = 0;
    unsigned int i;

    for (i = 0; i < length; i++)
        sum += x[i];

    return (sum & 0xFF) == 0;

}

static struct acpi_rsdp *findrsdp(void)
{

    char *signature = "RSD PTR ";
    unsigned int address;
    unsigned int ebda;

    for (address = 0x000E0000; address < 0x00100000; address += 0x10)
    {

        struct acpi_rsdp *rsdp = (struct acpi_rsdp *)address;

        if (buffer_match(rsdp->signature, signature, 8))
            return rsdp;

    }

    ebda = *pointer;
    ebda = ebda * 0x10 & 0x000FFFFF;

    for (address = ebda; address < ebda + 0x400; address += 0x10)
    {

        struct acpi_rsdp *rsdp = (struct acpi_rsdp *)address;

        if (buffer_match(rsdp->signature, signature, 8))
            return rsdp;

    }

    return 0;

}

static struct acpi_sdth *getsdth(struct acpi_rsdp *rsdp)
{

    if (rsdp->revision == 0)
    {

        if (validate(rsdp, sizeof (struct acpi_rsdp)))
            return (struct acpi_sdth *)((rsdp->rsdt[0] << 0) | (rsdp->rsdt[1] << 8) | (rsdp->rsdt[2] << 16) | (rsdp->rsdt[3] << 24));

    }

    if (rsdp->revision == 2)
    {

        struct acpi_xsdp *xsdp = (struct acpi_xsdp *)(rsdp + 1);

        if (validate(rsdp, sizeof (struct acpi_rsdp)) && validate(xsdp, sizeof (struct acpi_xsdp)))
            return (struct acpi_sdth *)((xsdp->xsdt[0] << 0) | (xsdp->xsdt[1] << 8) | (xsdp->xsdt[2] << 16) | (xsdp->xsdt[3] << 24));

    }

    return 0;

}

struct acpi_sdth *acpi_findheader(char *name)
{

    struct acpi_rsdp *rsdp = findrsdp();
    struct acpi_sdth *sdth = getsdth(rsdp);
    struct acpi_sdth **entries = (struct acpi_sdth **)(sdth + 1);
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

    if (rsdp)
    {

        struct acpi_sdth *sdth = getsdth(rsdp);

        if (sdth)
        {

            arch_kmap((unsigned int)sdth & 0xFFFFF000, (unsigned int)sdth & 0xFFFFF000, 0x00010000);
            /* Remove this mapping. Should not be needed. */
            arch_umap((unsigned int)sdth & 0xFFFFF000, (unsigned int)sdth & 0xFFFFF000, 0x00010000);

        }

    }

}

