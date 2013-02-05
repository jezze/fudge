#include <fudge/module.h>
#include <fudge/define.h>
#include <fudge/data/circular.h>
#include <system/system.h>
#include <base/base.h>
#include "uart.h"

static struct uart_device device1;
static struct uart_device device2;
static struct uart_device device3;
static struct uart_device device4;
static struct uart_driver driver;

void init()
{

    uart_init_device(&device1, UART_BASE1, UART_IRQ1);
    base_register_device(&device1.base);
    uart_init_device(&device2, UART_BASE2, UART_IRQ2);
    base_register_device(&device2.base);
    uart_init_device(&device3, UART_BASE3, UART_IRQ1);
    base_register_device(&device3.base);
    uart_init_device(&device4, UART_BASE4, UART_IRQ2);
    base_register_device(&device4.base);
    uart_init_driver(&driver);
    base_register_driver(&driver.base);

}

void destroy()
{

    base_unregister_driver(&driver.base);
    base_unregister_device(&device1.base);
    base_unregister_device(&device2.base);
    base_unregister_device(&device3.base);
    base_unregister_device(&device4.base);

}

