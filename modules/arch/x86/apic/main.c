#include <fudge/module.h>
#include <arch/x86/cpuid/cpuid.h>

void init()
{

    struct cpuid_data data;

    cpuid_get_data(CPUID_FEATURES0, &data);

    if (!(data.edx & CPUID_FEATURES01_APIC))
        return;

}

void destroy()
{

}

