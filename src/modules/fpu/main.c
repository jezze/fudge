#include <fudge.h>
#include <modules/cpuid/cpuid.h>
#include "fpu.h"

void module_init(void)
{

    struct cpuid_data data;

    if (!cpuid_exist())
        return;

    cpuid_getdata(CPUID_FEATURES0, &data);

    if (!(data.edx & CPUID_FEATURES01_FPU))
        return;

    fpu_enable();

}

