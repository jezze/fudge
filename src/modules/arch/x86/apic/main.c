#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/arch.h>
#include <modules/arch/x86/cpuid/cpuid.h>
#include <modules/arch/x86/msr/msr.h>
#include <modules/log/log.h>
#include "apic.h"

#define MSRBASE                         0x1B

static unsigned int apicbase;

static void apic_outd(unsigned int reg, unsigned int value)
{

    unsigned int *base = (unsigned int *)apicbase;

    *(base + reg) = value;

}

unsigned short apic_interrupt(struct cpu_general general, unsigned int index, unsigned int slave, struct cpu_interrupt interrupt)
{

    return arch_schedule(&general, &interrupt);

}

void module_init(void)
{

    struct cpuid_data data;
    struct msr_data msrdata;
    unsigned int count;
    char num[32];

    if (!cpuid_exist())
        return;

    if (!msr_exist())
        return;

    cpuid_getdata(CPUID_FEATURES0, &data);

    if (!(data.edx & CPUID_FEATURES01_APIC))
        return;

    msr_get(MSRBASE, &msrdata);

    apicbase = (msrdata.eax & 0xFFFFF000);

    msrdata.eax = apicbase | (1 << 11);
    msrdata.edx = 0;

    msr_set(MSRBASE, &msrdata);

    arch_setmap(6, apicbase, apicbase, 0x00400000);

    count = ascii_wzerovalue(num, 32, apicbase, 16, 8, 0);

    log_notify(LOG_INFO, count, num);
    log_notify(LOG_INFO, 1, "\n");
    log_notify(LOG_INFO, 2, "X\n");
    arch_setinterrupt(0xFF, apic_spurious);
    apic_outd(0x0F, 0xFF | (1 << 8));
    log_notify(LOG_INFO, 2, "X\n");

}

