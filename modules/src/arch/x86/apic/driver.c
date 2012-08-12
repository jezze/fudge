#include <memory.h>
#include <modules.h>
#include <arch/x86/isr.h>
#include <arch/x86/apic/apic.h>
#include <arch/x86/cpuid/cpuid.h>
#include <arch/x86/io/io.h>

static struct apic_routine routines[APIC_TABLE_SLOTS];

static unsigned int register_routine(struct apic_driver *driver, unsigned int index, struct modules_device *device, void (*callback)(struct modules_device *device))
{

    if (routines[index].device)
        return 0;

    routines[index].device = device;
    routines[index].callback = callback;

    return 1;

}

static unsigned int unregister_routine(struct apic_driver *driver, unsigned int index, struct modules_device *device)
{

    if (routines[index].device != device)
        return 0;

    routines[index].device = 0;
    routines[index].callback = 0;

    return 1;

}

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

static void start(struct modules_driver *self)
{

    struct cpuid_data data;
    unsigned int i;

    cpuid_get(CPUID_INSTRUCTION_FEATURES0, &data);

    if (!(data.edx & CPUID_FEATURES0_EDX_FLAG_APIC))
        return;

    for (i = 0; i < APIC_TABLE_SLOTS; i++)
        isr_register_routine(i + APIC_DATA_MASTERVECTOR, handle_interrupt);

    remap();
    enable();

}

void apic_driver_init(struct apic_driver *driver)
{

    memory_clear(driver, sizeof (struct apic_driver));

    modules_driver_init(&driver->base, APIC_DRIVER_TYPE, "apic", start, 0, 0);

    driver->register_routine = register_routine;
    driver->unregister_routine = unregister_routine;

}

