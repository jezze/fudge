#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/idt.h>
#include <kernel/x86/arch.h>
#include <modules/cpuid/cpuid.h>
#include <modules/msr/msr.h>
#include <modules/acpi/acpi.h>
#include <modules/apic/apic.h>

#define ROUTINES                        256
#define OVERRIDES                       16
#define MSR_APIC                        0x1B
#define IOAPIC_MASK                     (1 << 16)
#define MSIBASE                         0x10
#define MSIS                            16

static struct {unsigned int detected;} lapics[256];
static struct {unsigned int detected; unsigned int address; unsigned int gsibase;} ioapics[256];
static struct gdt_pointer *gdt = (struct gdt_pointer *)ARCH_GDT_BASE;
static struct idt_pointer *idt = (struct idt_pointer *)ARCH_IDT_BASE;
static void (*routines[ROUTINES])(unsigned int gsi);
static unsigned int overrides[OVERRIDES];
static unsigned int mmio;
static unsigned long ioapicbase;
static unsigned int redirections[OVERRIDES];
static struct spinlock spinlock;

static volatile unsigned long readio(unsigned long base, unsigned int offset)
{

    *(volatile unsigned long *)(base) = offset;

    return *(volatile unsigned long *)(base + 0x10);

}

static void writeio(unsigned long base, unsigned int offset, unsigned int value)
{

    *(volatile unsigned int *)(base) = offset;
    *(volatile unsigned int *)(base + 0x10) = value;

}

static void updatepin(unsigned int pin)
{

    unsigned int value = redirections[pin];

    if (!routines[overrides[pin]])
        value |= IOAPIC_MASK;

    spinlock_acquire(&spinlock);
    writeio(ioapicbase, 0x10 + pin * 2, value);
    spinlock_release(&spinlock);

}

static void updateirq(unsigned int irq)
{

    unsigned int pin;

    for (pin = 0; pin < OVERRIDES; pin++)
    {

        if (redirections[pin] && overrides[pin] == irq)
            updatepin(pin);

    }

}

void apic_debug_ioapic(void)
{

    struct acpi_madt *madt = (struct acpi_madt *)acpi_findheader("APIC");

    if (madt)
    {

        unsigned long madttable = (unsigned long)madt + sizeof (struct acpi_madt);
        unsigned long madtend = (unsigned long)madt + madt->base.length;

        while (madttable < madtend)
        {

            struct acpi_madt_entry *entry = (struct acpi_madt_entry *)madttable;

            if (entry->type == 1)
            {

                struct acpi_madt_ioapic *ioapic = (struct acpi_madt_ioapic *)entry;
                unsigned int ioapicid = readio(ioapic->address, 0);
                unsigned int ioapicversion = readio(ioapic->address, 1);
                unsigned int id = ((ioapicid >> 24) & 0x0F);
                unsigned int version = ioapicversion & 0xFF;
                unsigned int max = ((ioapicversion >> 16) & 0xFF) + 1;
                unsigned int j;

                debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "ioapic id %u version %u max %u", &id, &version, &max);

                for (j = 0; j < max; j++)
                {

                    unsigned int value0 = readio(ioapic->address, 0x10 + j * 2);
                    unsigned int value1 = readio(ioapic->address, 0x10 + j * 2 + 1);

                    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "ioapic redtbl %u %H8u:%H8u", &j, &value1, &value0);

                }

            }

            madttable += entry->length;

        }

    }

}

void apic_debug(void)
{

    struct acpi_madt *madt = (struct acpi_madt *)acpi_findheader("APIC");

    if (madt)
    {

        unsigned long madttable = (unsigned long)madt + sizeof (struct acpi_madt);
        unsigned long madtend = (unsigned long)madt + madt->base.length;

        while (madttable < madtend)
        {

            struct acpi_madt_entry *entry = (struct acpi_madt_entry *)madttable;

            if (entry->type == 0)
            {

                struct acpi_madt_lapic *lapic = (struct acpi_madt_lapic *)entry;

                debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "lapic processor %c id %c flags 0x%8Hu", &lapic->processor, &lapic->id, &lapic->flags);

            }

            if (entry->type == 1)
            {

                struct acpi_madt_ioapic *ioapic = (struct acpi_madt_ioapic *)entry;

                debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "ioapic id %c address 0x%8Hu gsibase %u", &ioapic->id, &ioapic->address, &ioapic->gsibase);

            }

            if (entry->type == 2)
            {

                struct acpi_madt_ioapic_iso *override = (struct acpi_madt_ioapic_iso *)entry;

                debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "ioapic override bus %c irq %c gsi %u flags %4Hh", &override->bus, &override->irq, &override->gsi, &override->flags);

            }

            if (entry->type == 4)
            {

                struct acpi_madt_lapic_nmi *nmi = (struct acpi_madt_lapic_nmi *)entry;

                debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "lapic nmi processor %c", &nmi->processor);

            }

            madttable += entry->length;

        }

    }

}

void apic_setupisrs(void)
{

    struct acpi_madt *madt = (struct acpi_madt *)acpi_findheader("APIC");

    if (madt)
    {

        unsigned long madttable = (unsigned long)madt + sizeof (struct acpi_madt);
        unsigned long madtend = (unsigned long)madt + madt->base.length;

        while (madttable < madtend)
        {

            struct acpi_madt_entry *entry = (struct acpi_madt_entry *)madttable;

            if (entry->type == 1)
            {

                struct acpi_madt_ioapic *ioapic = (struct acpi_madt_ioapic *)entry;
                unsigned int pin;

                if (ioapic->gsibase == 0)
                {

                    idt_setdescriptor(idt, 0x61, (void (*)(void))((unsigned long)apic_isr + 8 * 0x01), gdt_getselector(gdt, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
                    idt_setdescriptor(idt, 0x62, (void (*)(void))((unsigned long)apic_isr + 8 * 0x02), gdt_getselector(gdt, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
                    idt_setdescriptor(idt, 0x63, (void (*)(void))((unsigned long)apic_isr + 8 * 0x03), gdt_getselector(gdt, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
                    idt_setdescriptor(idt, 0x64, (void (*)(void))((unsigned long)apic_isr + 8 * 0x04), gdt_getselector(gdt, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
                    idt_setdescriptor(idt, 0x68, (void (*)(void))((unsigned long)apic_isr + 8 * 0x08), gdt_getselector(gdt, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
                    idt_setdescriptor(idt, 0x6A, (void (*)(void))((unsigned long)apic_isr + 8 * 0x0A), gdt_getselector(gdt, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
                    idt_setdescriptor(idt, 0x6B, (void (*)(void))((unsigned long)apic_isr + 8 * 0x0B), gdt_getselector(gdt, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
                    idt_setdescriptor(idt, 0x6C, (void (*)(void))((unsigned long)apic_isr + 8 * 0x0C), gdt_getselector(gdt, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
                    idt_setdescriptor(idt, 0x6E, (void (*)(void))((unsigned long)apic_isr + 8 * 0x0E), gdt_getselector(gdt, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
                    ioapicbase = ioapic->address;
                    redirections[0x01] = 0x61;
                    redirections[0x02] = 0x62;
                    redirections[0x03] = 0x63;
                    redirections[0x04] = 0x64;
                    redirections[0x08] = 0x68;
                    redirections[0x0A] = 0x6A | (1 << 15);
                    redirections[0x0B] = 0x6B | (1 << 15);
                    redirections[0x0C] = 0x6C;
                    redirections[0x0E] = 0x6E;

                    for (pin = 0; pin < OVERRIDES; pin++)
                    {

                        if (redirections[pin])
                            updatepin(pin);

                    }

                }

            }

            madttable += entry->length;

        }

    }

    idt_setdescriptor(idt, 0xFE, apic_test, gdt_getselector(gdt, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(idt, 0xFF, apic_spurious, gdt_getselector(gdt, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);

}

static void detect(void)
{

    struct acpi_madt *madt = (struct acpi_madt *)acpi_findheader("APIC");

    if (madt)
    {

        unsigned long madttable = (unsigned long)madt + sizeof (struct acpi_madt);
        unsigned long madtend = (unsigned long)madt + madt->base.length;

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
                ioapics[ioapic->id].gsibase = ioapic->gsibase;

            }

            if (entry->type == 2)
            {

                struct acpi_madt_ioapic_iso *override = (struct acpi_madt_ioapic_iso *)entry;

                if (override->gsi < 16)
                    overrides[override->gsi] = override->irq;

            }

            if (entry->type == 4)
            {

                /* local apic nmi */

            }

            madttable += entry->length;

        }

    }

}

static unsigned long apic_ind(unsigned long reg)
{

    volatile unsigned long *address = (volatile unsigned long *)(mmio + reg);

    return *address;

}

static void apic_outd(unsigned long reg, unsigned int value)
{

    volatile unsigned long *address = (volatile unsigned long *)(mmio + reg);

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

unsigned int apic_getid(void)
{

    return (apic_ind(APIC_REG_ID) >> 24);

}

unsigned short apic_interrupt(struct cpu_general general, unsigned int index, struct cpu_interrupt interrupt)
{

    unsigned int gsi = (index < OVERRIDES) ? overrides[index] : index;

    if (gsi < ROUTINES && routines[gsi])
        routines[gsi](gsi);

    apic_outd(APIC_REG_EOI, 0);

    return arch_resume(&general, &interrupt);

}

unsigned short apic_interruptnone(struct cpu_general general, struct cpu_interrupt interrupt)
{

    apic_outd(APIC_REG_EOI, 0);

    return arch_resume(&general, &interrupt);

}

unsigned int apic_setroutine(unsigned int irq, void (*routine)(unsigned int irq))
{

    if (irq >= ROUTINES)
        return 0;

    routines[irq] = routine;

    updateirq(irq);

    return 1;

}

unsigned int apic_setmsiroutine(void (*routine)(unsigned int irq))
{

    unsigned int irq;

    if (!mmio)
        return 0;

    for (irq = MSIBASE; irq < MSIBASE + MSIS; irq++)
    {

        if (!routines[irq])
        {

            idt_setdescriptor(idt, 0x60 + irq, (void (*)(void))((unsigned long)apic_isr + 8 * irq), gdt_getselector(gdt, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);

            routines[irq] = routine;

            return irq;

        }

    }

    return 0;

}

unsigned int apic_getmsiaddress(void)
{

    return 0xFEE00000 | (apic_getid() << 12);

}

unsigned int apic_getmsidata(unsigned int irq)
{

    return 0x60 + irq;

}

unsigned int apic_unsetroutine(unsigned int irq)
{

    if (irq >= ROUTINES)
        return 0;

    routines[irq] = 0;

    updateirq(irq);

    return 1;

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

    apic_outd(APIC_REG_ICR1, id << 24);
    apic_outd(APIC_REG_ICR0, value);

    while (apic_ind(APIC_REG_ICR0) & APIC_REG_ICR_STATUS_PENDING);

}

void module_init(void)
{

    struct cpuid_data data;
    struct msr_data msrdata;
    unsigned int i;

    if (!cpuid_exist())
        return;

    if (!msr_exist())
        return;

    cpuid_getdata(CPUID_FEATURES0, &data);

    for (i = 0; i < OVERRIDES; i++)
        overrides[i] = i;

/*
    if ((data.ecx & CPUID_FEATURES00_2XAPIC))
    {


    }
*/

    if ((data.edx & CPUID_FEATURES01_APIC))
    {

        msr_get(MSR_APIC, &msrdata);

        mmio = (msrdata.eax & 0xFFFFF000);

        arch_kmap(0xFEC00000, 0xFEC00000, 0x400000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);
        detect();

    }

}

