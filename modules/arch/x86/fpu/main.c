#include <fudge/module.h>
#include <arch/x86/cpuid/cpuid.h>
#include "fpu.h"

void init()
{

    struct cpuid_data data;

    cpuid_get_data(CPUID_FEATURES0, &data);

    if (!(data.edx & CPUID_FEATURES0_EDX_FPU))
        return;

    fpu_enable();

}

void destroy()
{

}

