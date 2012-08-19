#include <modules.h>
#include <arch/x86/cpuid/cpuid.h>

void init()
{

    if (!cpuid_check_cpuid())
        return;

}

void destroy()
{

}

