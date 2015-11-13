#include <fudge.h>
#include <modules/arch/x86/cpuid/cpuid.h>
#include "msr.h"

unsigned int msr_exist(void)
{

    struct cpuid_data data;

    if (!cpuid_exist())
        return 0;

    cpuid_getdata(CPUID_FEATURES0, &data);

    return data.edx & CPUID_FEATURES01_MSR;

}

void module_init(void)
{

}

