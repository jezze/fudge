#include <fudge.h>
#include <arch/x86/cpuid/cpuid.h>

void module_init()
{

    struct cpuid_data data;

    cpuid_getdata(CPUID_FEATURES0, &data);

    if (!(data.edx & CPUID_FEATURES01_APIC))
        return;

}

