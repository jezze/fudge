#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/idt.h>
#include <kernel/x86/tss.h>
#include <kernel/x86/arch.h>
#include "acpi.h"

static unsigned int sdt;

static unsigned int validate(void *address, unsigned int length)
{

    unsigned char *x = address;
    unsigned char sum = 0;
    unsigned int i;

    for (i = 0; i < length; i++)
        sum += x[i];

    return !sum;

}

static struct acpi_rsdp *findrsdp(void)
{

    char *signature = "RSD PTR ";
    unsigned int address;

    for (address = 0x000E0000; address < 0x00100000; address += 0x10)
    {

        struct acpi_rsdp *rsdp = (struct acpi_rsdp *)address;

        if (buffer_match(rsdp->signature, signature, 8))
            return rsdp;

    }

    return 0;

}

struct acpi_sdth *acpi_findheader(char *name)
{

    if (sdt)
    {

        struct acpi_sdth *sdth = (struct acpi_sdth *)sdt;
        struct acpi_sdth **entries = (struct acpi_sdth **)(sdth + 1);
        unsigned int total = (sdth->length - sizeof (struct acpi_sdth)) / 4;
        unsigned int i;

        for (i = 0; i < total; i++)
        {

            if (buffer_match(entries[i]->signature, name, 4))
                return entries[i];

        }

    }

    return 0;

}

void module_init(void)
{

    struct acpi_rsdp *rsdp = findrsdp();

    if (rsdp)
    {

        if (rsdp->revision == 0)
        {

            if (validate(rsdp, sizeof (struct acpi_rsdp)))
            {

                unsigned int address = (rsdp->rsdt[0] << 0) | (rsdp->rsdt[1] << 8) | (rsdp->rsdt[2] << 16) | (rsdp->rsdt[3] << 24);
                struct acpi_rsdt *rsdt = (struct acpi_rsdt *)address;

                arch_kmap(address, address, 0x00010000, MMAP_FLAG_WRITEABLE);
                /* Remove this mapping. Should not be needed. */
                arch_umap(address, address, 0x00010000, MMAP_FLAG_WRITEABLE);

                if (validate(rsdt, rsdt->base.length))
                    sdt = address;

            }

        }

        if (rsdp->revision == 2)
        {

            struct acpi_xsdp *xsdp = (struct acpi_xsdp *)(rsdp + 1);

            if (validate(rsdp, sizeof (struct acpi_rsdp)) && validate(xsdp, sizeof (struct acpi_xsdp)))
            {

                unsigned int address = (xsdp->xsdt[0] << 0) | (xsdp->xsdt[1] << 8) | (xsdp->xsdt[2] << 16) | (xsdp->xsdt[3] << 24);
                struct acpi_xsdt *xsdt = (struct acpi_xsdt *)address;

                arch_kmap(address, address, 0x00010000, MMAP_FLAG_WRITEABLE);
                /* Remove this mapping. Should not be needed. */
                arch_umap(address, address, 0x00010000, MMAP_FLAG_WRITEABLE);

                if (validate(xsdt, xsdt->base.length))
                    sdt = address;

            }

        }

    }

}

