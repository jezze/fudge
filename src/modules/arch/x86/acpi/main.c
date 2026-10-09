#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/arch.h>
#include "acpi.h"

static unsigned long sdt;
static unsigned int sdtentrysize;

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

    struct acpi_rsdp *firmware = (struct acpi_rsdp *)ARCH_FIRMWARE_ACPI;
    char *signature = "RSD PTR ";
    unsigned long address;

    if (buffer_match(firmware->signature, signature, 8))
        return firmware;

    arch_kmap(0x000E0000, 0x000E0000, 0x00020000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    for (address = 0x000E0000; address < 0x00100000; address += 0x10)
    {

        struct acpi_rsdp *rsdp = (struct acpi_rsdp *)address;

        if (buffer_match(rsdp->signature, signature, 8))
            return rsdp;

    }

    return 0;

}

static unsigned int getcount(void)
{

    struct acpi_sdth *sdth = (struct acpi_sdth *)sdt;

    return (sdth->length - sizeof (struct acpi_sdth)) / sdtentrysize;

}

static unsigned long getentry(unsigned int index)
{

    unsigned char *entries = (unsigned char *)((struct acpi_sdth *)sdt + 1);

    return *(unsigned int *)(entries + index * sdtentrysize);

}

struct acpi_sdth *acpi_findheader(char *name)
{

    if (sdt)
    {

        unsigned int i;

        for (i = 0; i < getcount(); i++)
        {

            struct acpi_sdth *entry = (struct acpi_sdth *)getentry(i);

            if (buffer_match(entry->signature, name, 4))
                return entry;

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

                unsigned long address = (rsdp->rsdt[0] << 0) | (rsdp->rsdt[1] << 8) | (rsdp->rsdt[2] << 16) | (rsdp->rsdt[3] << 24);
                struct acpi_rsdt *rsdt = (struct acpi_rsdt *)address;

                arch_kmap(address, address, 0x00010000, MMAP_FLAG_GLOBAL);

                if (validate(rsdt, rsdt->base.length))
                {

                    sdt = address;
                    sdtentrysize = 4;

                }

            }

        }

        if (rsdp->revision == 2)
        {

            struct acpi_xsdp *xsdp = (struct acpi_xsdp *)(rsdp + 1);

            if (validate(rsdp, sizeof (struct acpi_rsdp)) && validate(xsdp, sizeof (struct acpi_xsdp)))
            {

                unsigned long address = (xsdp->xsdt[0] << 0) | (xsdp->xsdt[1] << 8) | (xsdp->xsdt[2] << 16) | (xsdp->xsdt[3] << 24);
                struct acpi_xsdt *xsdt = (struct acpi_xsdt *)address;

                arch_kmap(address, address, 0x00010000, MMAP_FLAG_GLOBAL);

                if (validate(xsdt, xsdt->base.length))
                {

                    sdt = address;
                    sdtentrysize = 8;

                }

            }

        }

    }

    if (sdt)
    {

        unsigned int i;

        for (i = 0; i < getcount(); i++)
            arch_kmap(getentry(i), getentry(i), 0x00010000, MMAP_FLAG_GLOBAL);

    }

}

