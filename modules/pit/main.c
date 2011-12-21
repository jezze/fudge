#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/vfs.h>
#include <kernel/event.h>
#include <kernel/modules.h>
#include <kernel/kernel.h>
#include <modules/pit/pit.h>

static struct pit_device device;

static void handle_irq()
{

    struct pit_device *pit = &device;

    pit->jiffies += 1;

    event_handle(EVENT_IRQ_PIT);

}

void init()
{

    pit_device_init(&device);

    kernel_register_irq(0x00, handle_irq);

    modules_register_device(&device.base);

}

void destroy()
{

    kernel_unregister_irq(0x00);

    modules_unregister_device(&device.base);

}

