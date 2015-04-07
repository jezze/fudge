#include <fudge.h>
#include <arch/x86/cpuid/cpuid.h>
#include "fpu.h"

void module_init()
{

    struct cpuid_data data;

    cpuid_getdata(CPUID_FEATURES0, &data);

    if (!(data.edx & CPUID_FEATURES01_FPU))
        return;

    fpu_enable();

}

