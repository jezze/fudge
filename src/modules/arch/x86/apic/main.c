#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/idt.h>
#include <kernel/x86/tss.h>
#include <kernel/x86/arch.h>
#include <modules/arch/x86/cpuid/cpuid.h>
#include <modules/arch/x86/msr/msr.h>
#include <modules/arch/x86/acpi/acpi.h>
#include "apic.h"

#define MSR_APIC                        0x1B

static struct {unsigned int detected;} lapics[256];
static struct {unsigned int detected; unsigned int address;} ioapics[256];
static struct arch_gdt *gdt = (struct arch_gdt *)ARCH_GDTPHYSICAL;
static struct arch_idt *idt = (struct arch_idt *)ARCH_IDTPHYSICAL;
static unsigned int mmio;

static struct acpi_madt_ioapic_intsource *ioapic_intsource;

static unsigned int readio(unsigned int base, unsigned int offset)
{

    *(volatile unsigned int *)(base) = offset;

    return *(volatile unsigned int *)(base + 0x10);

}

/*
static void writeio(unsigned int base, unsigned int reg, unsigned int value)
{

    volatile unsigned int *ioregsel = (volatile unsigned int *)(base);
    volatile unsigned int *ioregwin = (volatile unsigned int *)(base + 0x10);

    *ioregsel = reg;
    *ioregwin = value;

}
*/

void apic_debug(void)
{

    unsigned int i;

    for (i = 0; i < 256; i++)
    {

        if (ioapics[i].detected)
        {

            unsigned int ioapicid = readio(ioapics[i].address, 0);
            unsigned int ioapicversion = readio(ioapics[i].address, 1);
            unsigned int version = ioapicversion & 0xFF;
            unsigned int max = ((ioapicversion >> 8) & 0xFF) + 1;
            unsigned int j;

            DEBUG_FMT1(DEBUG_INFO, "ioapic mmio %8Hu", &mmio);
            DEBUG_FMT1(DEBUG_INFO, "ioapic address %8Hu", &ioapics[i].address);
            DEBUG_FMT1(DEBUG_INFO, "ioapic reg0 %u", &ioapicid);
            DEBUG_FMT1(DEBUG_INFO, "ioapic reg1 %u", &ioapicversion);
            DEBUG_FMT1(DEBUG_INFO, "ioapic version %u", &version);
            DEBUG_FMT1(DEBUG_INFO, "ioapic max %u", &max);

            if (ioapics[i].address == (unsigned int)0xFEC00000)
            {

                DEBUG_FMT0(DEBUG_INFO, "match");

            }

            for (j = 0; j < 48; j++)
            {

                unsigned int value0 = readio(ioapics[i].address, 0x10 + i * 2);
                unsigned int value1 = readio(ioapics[i].address, 0x10 + i * 2 + 1);
                unsigned int vector = value0 & 0xFF;

                DEBUG_FMT2(DEBUG_INFO, "ioapic regs %H8u:%H8u", &value1, &value0);
                DEBUG_FMT1(DEBUG_INFO, "ioapic vector %u", &vector);

            }

        }

    }

}

static void detect(void)
{

    struct acpi_madt *madt = (struct acpi_madt *)acpi_findheader("APIC");

    if (madt)
    {

        unsigned int madttable = (unsigned int)madt + sizeof (struct acpi_madt);
        unsigned int madtend = (unsigned int)madt + madt->base.length;

        if (madt->flags & 0x01)
        {

            /* pic_disable(); */

        }

        while (madttable < madtend)
        {

            struct acpi_madt_entry *entry = (struct acpi_madt_entry *)madttable;

            if (entry->type == 0)
            {

                struct acpi_madt_lapic *lapic = (struct acpi_madt_lapic *)entry;

                if ((lapic->flags & 0x01) || (lapic->flags & 0x02))
                {

                    lapics[lapic->id].detected = 1;

                }

            }

            if (entry->type == 1)
            {

                struct acpi_madt_ioapic *ioapic = (struct acpi_madt_ioapic *)entry;

                ioapics[ioapic->id].detected = 1;
                ioapics[ioapic->id].address = ioapic->address;

                /*arch_mapuncached(10 + ioapic->id, ioapics[ioapic->id].address, ioapics[ioapic->id].address, 0x1000);*/

            }

            if (entry->type == 2)
            {

                if (ioapic_intsource == 0)
                    ioapic_intsource = (struct acpi_madt_ioapic_intsource *)entry;

            }

            /*
            if (entry->type == 4)
            {

            }

            */

            madttable += entry->length;

        }

    }

}

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

unsigned int apic_checklapic(unsigned int id)
{

    return lapics[id].detected;

}

unsigned int apic_checkioapic(unsigned int id)
{

    return ioapics[id].detected;

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

unsigned short apic_interrupt(struct cpu_general general, struct cpu_interrupt interrupt)
{

    apic_outd(APIC_REG_EOI, 0);

    return arch_resume(&general, &interrupt);

}

unsigned int apic_setroutine(unsigned int irq, void (*routine)(unsigned int irq))
{

    return 0;

}

unsigned int apic_unsetroutine(unsigned int irq)
{

    return 0;

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

        msr_get(MSR_APIC, &msrdata);

        mmio = (msrdata.eax & 0xFFFFF000);

        arch_mapuncached(7, mmio, mmio, 0x400000);
        detect();
        idt_setdescriptor(&idt->pointer, 0xFE, apic_test, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
        idt_setdescriptor(&idt->pointer, 0xFF, apic_spurious, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);

    }

}

