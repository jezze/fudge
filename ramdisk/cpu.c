#include <lib/types.h>
#include <lib/string.h>
#include <kernel/syscall.h>
#include "cpu.h"

void main()
{

    if (!cpu_check_cpuid())
        return;

    char vendor[16];
    vendor[12] = '\0';

    cpu_get_vendor(vendor);

    call_puts("Processor: ");
    call_puts(vendor);
    call_puts("\n");

}

