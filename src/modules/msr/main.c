#include <fudge.h>
#include <modules/cpuid/cpuid.h>
#include "msr.h"

unsigned int msr_exist(void)
{

    if (cpuid_exist())
    {

        struct cpuid_data data;

        cpuid_getdata(CPUID_FEATURES0, &data);

        return data.edx & CPUID_FEATURES01_MSR;

    }

    return 0;

}

void module_init(void)
{

}

