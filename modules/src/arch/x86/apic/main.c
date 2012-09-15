#include <memory.h>
#include <base/base.h>
#include <arch/x86/idt.h>
#include <arch/x86/isr.h>
#include <arch/x86/apic/apic.h>
#include <arch/x86/cpuid/cpuid.h>
#include <arch/x86/io/io.h>

static struct apic_routine routines[APIC_TABLE_SLOTS];

static void raise(unsigned int index)
{

    if (!routines[index].callback)
        return;

    routines[index].callback(routines[index].device);

}

static void remap()
{

    io_outb(APIC_MASTER_COMMAND, APIC_COMMAND_CONFIG);
    io_outb(APIC_MASTER_DATA, APIC_DATA_MASTERVECTOR);
    io_outb(APIC_MASTER_DATA, 0x04);
    io_outb(APIC_MASTER_DATA, APIC_DATA_8086);

    io_outb(APIC_SLAVE_COMMAND, APIC_COMMAND_CONFIG);
    io_outb(APIC_SLAVE_DATA, APIC_DATA_SLAVEVECTOR);
    io_outb(APIC_SLAVE_DATA, 0x02);
    io_outb(APIC_SLAVE_DATA, APIC_DATA_8086);

}

static void enable()
{

    io_outb(APIC_MASTER_DATA, 0x00);
    io_outb(APIC_SLAVE_DATA, 0x00);

}

static void reset(unsigned int slave)
{

    if (slave)
        io_outb(APIC_SLAVE_COMMAND, APIC_COMMAND_EOI);

    io_outb(APIC_MASTER_COMMAND, APIC_COMMAND_EOI);

}

static void handle_interrupt(struct isr_registers *registers)
{

    raise(registers->index - APIC_DATA_MASTERVECTOR);
    reset(registers->extra);

}

unsigned int apic_register_routine(unsigned int index, struct base_device *device, void (*callback)(struct base_device *device))
{

    if (routines[index].device)
        return 0;

    routines[index].device = device;
    routines[index].callback = callback;

    return 1;

}

unsigned int apic_unregister_routine(unsigned int index, struct base_device *device)
{

    if (routines[index].device != device)
        return 0;

    routines[index].device = 0;
    routines[index].callback = 0;

    return 1;

}

void init()
{

    struct cpuid_data data;
    unsigned int i;

    cpuid_get(CPUID_INSTRUCTION_FEATURES0, &data);

    if (!(data.edx & CPUID_FEATURES0_EDX_FLAG_APIC))
        return;

    idt_set_entry(0x20, apic_routine00, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x21, apic_routine01, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x22, apic_routine02, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x23, apic_routine03, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x24, apic_routine04, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x25, apic_routine05, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x26, apic_routine06, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x27, apic_routine07, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x28, apic_routine08, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x29, apic_routine09, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x2A, apic_routine0A, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x2B, apic_routine0B, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x2C, apic_routine0C, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x2D, apic_routine0D, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x2E, apic_routine0E, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x2F, apic_routine0F, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);

    for (i = 0; i < APIC_TABLE_SLOTS; i++)
        isr_set_routine(i + APIC_DATA_MASTERVECTOR, handle_interrupt);

    remap();
    enable();

}

void destroy()
{

}

