#include <fudge.h>
#include <arch/x86/cpuid/cpuid.h>
#include "msr.h"

void init()
{

    struct cpuid_data data;

    cpuid_getdata(CPUID_FEATURES0, &data);

    if (!(data.edx & CPUID_FEATURES01_MSR))
        return;

}

void destroy()
{

}

