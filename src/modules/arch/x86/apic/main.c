#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/arch.h>
#include <modules/arch/x86/cpuid/cpuid.h>
#include <modules/arch/x86/msr/msr.h>
#include "apic.h"

#define REGISTERID                      0x0020
#define REGISTERVERSION                 0x0030
#define REGISTERTPR                     0x0080
#define REGISTERAPR                     0x0090
#define REGISTERCPR                     0x00A0
#define REGISTEREOI                     0x00B0
#define REGISTERREMOTEREAD              0x00C0
#define REGISTERLOGICDEST               0x00D0
#define REGISTERDESTFORMAT              0x00E0
#define REGISTERSV                      0x00F0
#define REGISTERISR0                    0x0100
#define REGISTERISR1                    0x0110
#define REGISTERISR2                    0x0120
#define REGISTERISR3                    0x0130
#define REGISTERISR4                    0x0140
#define REGISTERISR5                    0x0150
#define REGISTERISR6                    0x0160
#define REGISTERISR7                    0x0170
#define REGISTERTMR0                    0x0180
#define REGISTERTMR1                    0x0190
#define REGISTERTMR2                    0x01A0
#define REGISTERTMR3                    0x01B0
#define REGISTERTMR4                    0x01C0
#define REGISTERTMR5                    0x01D0
#define REGISTERTMR6                    0x01E0
#define REGISTERTMR7                    0x01F0
#define REGISTERIRR0                    0x0200
#define REGISTERIRR1                    0x0210
#define REGISTERIRR2                    0x0220
#define REGISTERIRR3                    0x0230
#define REGISTERIRR4                    0x0240
#define REGISTERIRR5                    0x0250
#define REGISTERIRR6                    0x0260
#define REGISTERIRR7                    0x0270
#define REGISTERERRORSTATUS             0x0280
#define REGISTERLVTCMCI                 0x02F0
#define REGISTERICR0                    0x0300
#define REGISTERICR1                    0x0310
#define REGISTERLVTTIMER                0x0320
#define REGISTERLVTTHERMAL              0x0330
#define REGISTERLVTPERF                 0x0340
#define REGISTERLVTLINT0                0x0350
#define REGISTERLVTLINT1                0x0360
#define REGISTERLVTERROR                0x0370
#define REGISTERINITIALCOUNT            0x0380
#define REGISTERCURRENTCOUNT            0x0390
#define REGISTERDIVIDECONFIG            0x03E0

static unsigned int mmio;

static void apic_outd(unsigned int reg, unsigned int value)
{

    unsigned int *base = (unsigned int *)mmio;

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

    if (!cpuid_exist())
        return;

    if (!msr_exist())
        return;

    cpuid_getdata(CPUID_FEATURES0, &data);

    if (!(data.edx & CPUID_FEATURES01_APIC))
        return;

    msr_get(MSR_LAPIC, &msrdata);

    mmio = (msrdata.eax & 0xFFFFF000);

    msrdata.eax = mmio | (1 << 11);
    msrdata.edx = 0;

    msr_set(MSR_LAPIC, &msrdata);

    arch_setmap(6, mmio, mmio, 0x1000);
    arch_setinterrupt(0xFF, apic_spurious);
    apic_outd(REGISTERSV, 0xFF | (1 << 8));

}

