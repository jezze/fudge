#include <fudge/module.h>
#include <base/base.h>
#include "uart.h"

void uart_init_device(struct uart_device *device, unsigned int port, unsigned int irq, char *name)
{

    memory_clear(device, sizeof (struct uart_device));
    base_init_device(&device->base, UART_DEVICE_TYPE, irq, name, 0);

    device->port = port;

}

