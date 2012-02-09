#include <lib/memory.h>
#include <kernel/log.h>
#include <kernel/mmu.h>
#include <modules/acpi/acpi.h>

static struct mmu_memory acpiMemory;

static struct acpi_sdth *get_table(struct acpi_sdth *sdt, char *name)
{

    void **tables = (void *)sdt + sizeof (struct acpi_sdth);
    unsigned int entries = (sdt->length - sizeof (struct acpi_sdth)) / 4;

    unsigned int i;

    for (i = 0; i < entries; i++)
    {

        struct acpi_sdth *current = tables[i];

        if (!memory_compare(current->signature, name, 4))
            return current;

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

void init()
{

    struct acpi_rsdp *rsdp = get_rsdp();

    if (!rsdp)
        return;

    log_write("[acpi] RSDP Address: 0x%x\n", rsdp);
    log_write("[acpi] RSDP Revision: %d.0\n", rsdp->revision + 1);

    struct mmu_memory *memory = &acpiMemory;

    mmu_memory_init(memory, (void *)rsdp->rsdt, (void *)rsdp->rsdt, 0x10000); 
    mmu_map_kernel_memory(memory);
    mmu_reload_memory();

    struct acpi_madt *madt = (struct acpi_madt *)get_table((struct acpi_sdth *)rsdp->rsdt, "APIC");

    if (madt)
        log_write("[acpi] MADT signature: %c%c%c%c\n", madt->base.signature[0], madt->base.signature[1], madt->base.signature[2], madt->base.signature[3]);

}

void destroy()
{

}

