#include <modules/modules.h>
#include <arch/x86/uart/uart.h>

static struct uart_device device1;
static struct uart_device device2;
static struct uart_device device3;
static struct uart_device device4;
static struct uart_driver driver;

void init()
{

    uart_device_init(&device1, UART_BASE1, UART_IRQ1);
    uart_device_init(&device2, UART_BASE2, UART_IRQ2);
    uart_device_init(&device3, UART_BASE3, UART_IRQ1);
    uart_device_init(&device4, UART_BASE4, UART_IRQ2);
    uart_driver_init(&driver);

    modules_register_device(&device1.base);
    modules_register_device(&device2.base);
    modules_register_device(&device3.base);
    modules_register_device(&device4.base);
    modules_register_driver(&driver.base);

}

void destroy()
{

    modules_unregister_driver(&driver.base);
    modules_unregister_device(&device1.base);
    modules_unregister_device(&device2.base);
    modules_unregister_device(&device3.base);
    modules_unregister_device(&device4.base);

}

