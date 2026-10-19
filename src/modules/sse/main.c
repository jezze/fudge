#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <modules/cpuid/cpuid.h>

static unsigned int supported;

void sse_setup(void)
{

    if (!supported)
        return;

    cpu_setcr0((cpu_getcr0() & ~CPU_CR0_EM) | CPU_CR0_MP);
    cpu_setcr4(cpu_getcr4() | CPU_CR4_OSFXSR | CPU_CR4_OSMMEXCPT);

}

void module_init(void)
{

    struct cpuid_data data;

    if (!cpuid_exist())
        return;

    cpuid_getdata(CPUID_FEATURES0, &data);

    supported = (data.edx & CPUID_FEATURES01_FXSR) && (data.edx & CPUID_FEATURES01_SSE) && (data.edx & CPUID_FEATURES01_SSE2);

    sse_setup();

}

