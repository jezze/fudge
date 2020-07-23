#include <fudge.h>
#include <kernel.h>
#include <modules/arch/x86/cpuid/cpuid.h>
#include <modules/arch/x86/msr/msr.h>

#define MSRPAT                          0x277

void pat_setup(void)
{

    struct msr_data msrdata;

    msr_get(MSRPAT, &msrdata);

    msrdata.eax &= 0xFFFF00FF;
    msrdata.eax |= 0x00000100;

    msr_set(MSRPAT, &msrdata);

}

void module_init(void)
{

    struct cpuid_data data;

    if (!cpuid_exist())
        return;

    if (!msr_exist())
        return;

    cpuid_getdata(CPUID_FEATURES0, &data);

    if ((data.edx & CPUID_FEATURES01_PAT))
    {

        pat_setup();

    }

}

