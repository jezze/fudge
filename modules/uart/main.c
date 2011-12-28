#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/modules.h>
#include <kernel/kernel.h>
#include <modules/uart/uart.h>

static struct uart_device device1;
static struct uart_driver driver;

static void handle_device1_irq()
{

    struct uart_device *device = &device1;

    char c = device1.read(device);

    driver.buffer.putc(&driver.buffer, &c);

}

void init()
{

    uart_device_init(&device1, UART_BASE1);
    modules_register_device(&device1.base);

    uart_driver_init(&driver);
    modules_register_driver(&driver.base);
    modules_attach(&driver.base);

    kernel_register_irq(0x04, handle_device1_irq);

}

void destroy()
{

    kernel_unregister_irq(0x04);

    modules_unregister_driver(&driver.base);
    modules_unregister_device(&device1.base);

}

