#include <lib/types.h>
#include <lib/string.h>
#include <kernel/screen.h>
#include <kernel/cpu.h>

void cpu_init()
{

    if (!cpu_check_cpuid())
        return;

    char vendor[13];
    vendor[12] = '\0';

    cpu_get_vendor(vendor);

    screen_puts("Processor: ");
    screen_puts(vendor);
    screen_putc('\n');

}

