#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/modules.h>
#include <kernel/kernel.h>
#include <modules/uart/uart.h>

static struct uart_device device1;

static void handle_device1_irq()
{

    struct uart_device *device = &device1;

    char c = device1.read(device);

    device->buffer.putc(&device->buffer, &c);

}

void init()
{

    uart_device_init(&device1, UART_BASE1);

    kernel_register_irq(0x04, handle_device1_irq);

    modules_register_device(&device1.base);

}

void destroy()
{

    kernel_unregister_irq(0x04);

    modules_unregister_device(&device1.base);

}

