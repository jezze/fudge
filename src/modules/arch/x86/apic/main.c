#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/idt.h>
#include <kernel/x86/tss.h>
#include <kernel/x86/arch.h>
#include <modules/arch/x86/cpuid/cpuid.h>
#include <modules/arch/x86/msr/msr.h>
#include <modules/arch/x86/pic/pic.h>
#include "apic.h"

#define MSR_LAPIC                       0x1B

static struct arch_gdt *gdt = (struct arch_gdt *)ARCH_GDTPHYSICAL;
static struct arch_idt *idt = (struct arch_idt *)ARCH_IDTPHYSICAL;
static unsigned int mmio;

static unsigned int apic_ind(unsigned int reg)
{

    volatile unsigned int *address = (volatile unsigned int *)(mmio + reg);

    return *address;

}

static void apic_outd(unsigned int reg, unsigned int value)
{

    volatile unsigned int *address = (volatile unsigned int *)(mmio + reg);

    *address = value;

}

/* TODO: Remove changing of CR3 register. This is a workaround because existing tasks cant access apic mapped register. */
unsigned int apic_getid(void)
{

    unsigned int directory = cpu_getcr3();
    unsigned int id;

    cpu_setcr3(ARCH_KERNELMMUPHYSICAL);

    id = apic_ind(APIC_REG_ID) >> 24;

    cpu_setcr3(directory);

    return id;
}

/* TODO: Remove changing of CR3 register. This is a workaround because existing tasks cant access apic mapped register. */
unsigned short apic_interrupt(struct cpu_general general, struct cpu_interrupt interrupt)
{

    unsigned int directory = cpu_getcr3();

    cpu_setcr3(ARCH_KERNELMMUPHYSICAL);
    apic_outd(APIC_REG_EOI, 0);
    cpu_setcr3(directory);

    return arch_resume(&general, &interrupt);

}

void apic_setup_ap(void)
{

    apic_outd(APIC_REG_TPR, 0x00000000);
    apic_outd(APIC_REG_LDR, 0x00000000);
    apic_outd(APIC_REG_DFR, 0xFFFFFFFF);
    apic_outd(APIC_REG_SV, (1 << 8) | 0xFF);
    apic_outd(APIC_REG_LVTCMCI, 0x00010000);
    apic_outd(APIC_REG_LVTTIMER, 0x00010000);
    apic_outd(APIC_REG_LVTPERF, 0x00010000);
    apic_outd(APIC_REG_LVTLINT0, 0x00010000);
    apic_outd(APIC_REG_LVTLINT1, 0x00010000);
    apic_outd(APIC_REG_LVTERROR, 0x00010000);

}

void apic_setup_bp(void)
{

    apic_outd(APIC_REG_TPR, 0x00000000);
    apic_outd(APIC_REG_LDR, 0x00000000);
    apic_outd(APIC_REG_DFR, 0xFFFFFFFF);
    apic_outd(APIC_REG_SV, (1 << 8) | 0xFF);
    apic_outd(APIC_REG_LVTCMCI, 0x00010000);
    apic_outd(APIC_REG_LVTTIMER, 0x00010000);
    apic_outd(APIC_REG_LVTPERF, 0x00010000);
    apic_outd(APIC_REG_LVTLINT0, 0x00000700);
    apic_outd(APIC_REG_LVTLINT1, 0x00000400);
    apic_outd(APIC_REG_LVTERROR, 0x00010000);

}

void apic_sendint(unsigned int id, unsigned int value)
{

    unsigned int directory = cpu_getcr3();

    cpu_setcr3(ARCH_KERNELMMUPHYSICAL);

    apic_outd(APIC_REG_ICR1, id << 24);
    apic_outd(APIC_REG_ICR0, value);

    while (apic_ind(APIC_REG_ICR0) & APIC_REG_ICR_STATUS_PENDING);

    cpu_setcr3(directory);

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

/*
    if ((data.ecx & CPUID_FEATURES00_2XAPIC))
    {


    }
*/

    if ((data.edx & CPUID_FEATURES01_APIC))
    {

        msr_get(MSR_LAPIC, &msrdata);

        mmio = (msrdata.eax & 0xFFFFF000);

        arch_setmap(7, mmio, mmio, 0x1000);
        idt_setdescriptor(&idt->pointer, 0xFE, apic_test, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
        idt_setdescriptor(&idt->pointer, 0xFF, apic_spurious, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);

    }

}

