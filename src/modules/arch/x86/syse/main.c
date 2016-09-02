#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/arch.h>
#include <modules/arch/x86/msr/msr.h>
#include "syse.h"

void module_init(void)
{

    struct msr_data msrdata;

    msrdata.eax = 8;
    msrdata.edx = 0;

    msr_set(0x174, &msrdata);

    msrdata.eax = 0x00400000;
    msrdata.edx = 0;

    msr_set(0x175, &msrdata);

    msrdata.eax = (unsigned int)&syse_interrupt;
    msrdata.edx = 0;

    msr_set(0x176, &msrdata);

}

