#include <kernel/irq.h>
#include <kernel/modules.h>
#include <modules/uart/uart.h>

static void handle_irq(struct modules_device *self)
{

    struct uart_device *device = (struct uart_device *)self;
    struct uart_driver *driver = (struct uart_driver *)self->driver;

    char c = device->read(device);

    driver->buffer.putc(&driver->buffer, &c);

}

static struct uart_device device1;
static struct uart_device device2;
static struct uart_device device3;
static struct uart_device device4;
static struct uart_driver driver;

void init()
{

    uart_device_init(&device1, UART_BASE1, 0x04);
    uart_device_init(&device2, UART_BASE2, 0x03);
    uart_device_init(&device3, UART_BASE3, 0x04);
    uart_device_init(&device4, UART_BASE4, 0x03);
    uart_driver_init(&driver);

    modules_register_device(&device1.base);
    modules_register_device(&device2.base);
    modules_register_device(&device3.base);
    modules_register_device(&device4.base);
    modules_register_driver(&driver.base);

    irq_register_routine(0x04, &device1.base, handle_irq);
    irq_register_routine(0x03, &device2.base, handle_irq);

}

void destroy()
{

    irq_unregister_routine(0x04, &device1.base);
    irq_unregister_routine(0x03, &device2.base);

    modules_unregister_driver(&driver.base);
    modules_unregister_device(&device1.base);
    modules_unregister_device(&device2.base);
    modules_unregister_device(&device3.base);
    modules_unregister_device(&device4.base);

}

