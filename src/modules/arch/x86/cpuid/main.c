#include <fudge.h>
#include <kernel/x86/cpu.h>
#include "cpuid.h"

void module_init(void)
{

    if (!(cpu_geteflags() & CPU_FLAGS_ID))
        return;

    if (!cpuid_check())
        return;

}

