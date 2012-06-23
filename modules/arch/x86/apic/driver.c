#include <lib/io.h>
#include <lib/memory.h>
#include <kernel/kernel.h>
#include <kernel/modules.h>
#include <kernel/arch/x86/isr.h>
#include <modules/apic/apic.h>

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

    io_outb(0x20, 0x11);
    io_outb(0xA0, 0x11);
    io_outb(0x21, 0x20);
    io_outb(0xA1, 0x28);
    io_outb(0x21, 0x04);
    io_outb(0xA1, 0x02);
    io_outb(0x21, 0x01);
    io_outb(0xA1, 0x01);
    io_outb(0x21, 0x00);
    io_outb(0xA1, 0x00);

}

static void reset(unsigned int slave)
{

    if (slave)
        io_outb(0xA0, 0x20);

    io_outb(0x20, 0x20);

}

static void handle_interrupt(struct kernel_context *context, struct isr_cpu_registers *registers)
{

    raise(registers->index - 0x20);
    reset(registers->error);

}

static void start(struct modules_driver *self)
{

    remap();

    isr_register_routine(ISR_INDEX_PIT, handle_interrupt);
    isr_register_routine(ISR_INDEX_KBD, handle_interrupt);
    isr_register_routine(ISR_INDEX_CASCADE, handle_interrupt);
    isr_register_routine(ISR_INDEX_COM2, handle_interrupt);
    isr_register_routine(ISR_INDEX_COM1, handle_interrupt);
    isr_register_routine(ISR_INDEX_SOUND, handle_interrupt);
    isr_register_routine(ISR_INDEX_SDA, handle_interrupt);
    isr_register_routine(ISR_INDEX_PP, handle_interrupt);
    isr_register_routine(ISR_INDEX_RTC, handle_interrupt);
    isr_register_routine(ISR_INDEX_MOUSE, handle_interrupt);
    isr_register_routine(ISR_INDEX_FPU, handle_interrupt);
    isr_register_routine(ISR_INDEX_ATAP, handle_interrupt);
    isr_register_routine(ISR_INDEX_ATAS, handle_interrupt);

}

void apic_driver_init(struct apic_driver *driver)
{

    memory_clear(driver, sizeof (struct apic_driver));

    modules_driver_init(&driver->base, APIC_DRIVER_TYPE, "apic", start, 0, 0);

    driver->register_routine = register_routine;
    driver->unregister_routine = unregister_routine;

}

