#include <fudge/module.h>
#include <fudge/define.h>
#include <fudge/data/circular.h>
#include <system/system.h>
#include <base/base.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/io/io.h>
#include "uart.h"

static void handle_irq(struct base_device *device)
{

    struct uart_device *uartDevice = (struct uart_device *)device;
    struct uart_driver *driver = (struct uart_driver *)device->driver;
    char data = uart_device_read(uartDevice);

    circular_stream_write(&driver->stream, 1, &data);

}

static void attach(struct base_device *device)
{

    struct uart_device *uartDevice = (struct uart_device *)device;

    pic_set_routine(device->irq, device, handle_irq);
    io_outb(uartDevice->port + UART_IER, 0x00);
    io_outb(uartDevice->port + UART_LCR, 0x80);
    io_outb(uartDevice->port + UART_THR, 0x03);
    io_outb(uartDevice->port + UART_IER, 0x00);
    io_outb(uartDevice->port + UART_LCR, 0x03);
    io_outb(uartDevice->port + UART_FCR, 0xC7);
    io_outb(uartDevice->port + UART_MCR, 0x0B);
    io_outb(uartDevice->port + UART_IER, 0x01);

}

static unsigned int check(struct base_device *device)
{

    return device->type == UART_DEVICE_TYPE;

}

void uart_init_driver(struct uart_driver *driver)
{

    memory_clear(driver, sizeof (struct uart_driver));
    base_init_driver(&driver->base, "uart", 0, check, attach);

}

