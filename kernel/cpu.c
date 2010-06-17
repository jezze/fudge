#include <lib/types.h>
#include <lib/string.h>
#include <kernel/cpu.h>

void cpu_init()
{

    if (cpu_check_cpuid())
        screen_puts("CPUID exist\n");

}

