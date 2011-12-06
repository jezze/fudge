#include <lib/memory.h>
#include <kernel/arch/x86/acpi.h>

static void *get_rsdp()
{

    char *signature = "RSD PTR ";
    void *rsdp;

    for (rsdp = (void *)0x000E0000; rsdp < (void *)0x000FFFFF; rsdp += 0x10)
    {

        if (!memory_compare(rsdp, signature, 8))
            return rsdp;

    }

    return 0;

}

void acpi_init()
{

    struct acpi_rsdp *rsdp = get_rsdp();

    if (!rsdp)
        return;

}

