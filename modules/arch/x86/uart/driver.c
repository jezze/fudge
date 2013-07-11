#include <fudge/module.h>
#include <fudge/define.h>
#include <fudge/data/circular.h>
#include <base/base.h>
#include <terminal/terminal.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/io/io.h>
#include "uart.h"

enum uart_ier
{

    UART_IER_NULL                       = 0,
    UART_IER_RECEIVE                    = (1 << 0),
    UART_IER_TRANSMIT                   = (1 << 1),
    UART_IER_LINE                       = (1 << 2),
    UART_IER_MODEM                      = (1 << 3),
    UART_IER_SLEEP                      = (1 << 4),
    UART_IER_POWER                      = (1 << 5)

};

enum uart_iir
{

    UART_IIR_PENDING                    = (1 << 0),
    UART_IIR_MODEMSTATUS                = (0 << 1),
    UART_IIR_TRANSMIT                   = (1 << 1),
    UART_IIR_RECEIVE                    = (2 << 1),
    UART_IIR_LINE                       = (3 << 1),
    UART_IIR_TIMEOUT                    = (6 << 1),
    UART_IIR_LARGE                      = (1 << 5),
    UART_IIR_NOFIFO                     = (1 << 6),
    UART_IIR_ERRORFIFO                  = (2 << 6),
    UART_IIR_FIFO                       = (3 << 6)

};

enum uart_fcr
{

    UART_FCR_ENABLE                     = (1 << 0),
    UART_FCR_RECEIVE                    = (1 << 1),
    UART_FCR_TRANSMIT                   = (1 << 2),
    UART_FCR_SELECT                     = (1 << 3),
    UART_FCR_LARGE                      = (1 << 5),
    UART_FCR_SIZE0                      = (0 << 6),
    UART_FCR_SIZE1                      = (1 << 6),
    UART_FCR_SIZE2                      = (2 << 6),
    UART_FCR_SIZE3                      = (3 << 6)

};

enum uart_lcr
{

    UART_LCR_5BITS                      = (0 << 0),
    UART_LCR_6BITS                      = (1 << 0),
    UART_LCR_7BITS                      = (2 << 0),
    UART_LCR_8BITS                      = (3 << 0),
    UART_LCR_1STOP                      = (0 << 2),
    UART_LCR_2STOP                      = (1 << 2),
    UART_LCR_NOPARITY                   = (0 << 3),
    UART_LCR_ODDPARITY                  = (1 << 3),
    UART_LCR_EVENPARITY                 = (3 << 3),
    UART_LCR_MARKPARITY                 = (5 << 3),
    UART_LCR_SPACEPARITY                = (7 << 3),
    UART_LCR_BREAK                      = (1 << 6),
    UART_LCR_LATCH                      = (1 << 7)

};

enum uart_mcr
{

    UART_MCR_READY                      = (1 << 0),
    UART_MCR_REQUEST                    = (1 << 1),
    UART_MCR_AUX1                       = (1 << 2),
    UART_MCR_AUX2                       = (1 << 3),
    UART_MCR_LOOPBACK                   = (1 << 4),
    UART_MCR_AUTOFLOW                   = (1 << 5)

};

enum uart_lsr
{

    UART_LSR_READY                      = (1 << 0),
    UART_LSR_OVERRUN                    = (1 << 1),
    UART_LSR_PARITY                     = (1 << 2),
    UART_LSR_FRAMING                    = (1 << 3),
    UART_LSR_BREAK                      = (1 << 4),
    UART_LSR_TRANSMIT                   = (1 << 5),
    UART_LSR_RECEIVE                    = (1 << 6),
    UART_LSR_FIFO                       = (1 << 7)

};

enum uart_msr
{

    UART_MSR_DELTACLEAR                 = (1 << 0),
    UART_MSR_DELTAREADY                 = (1 << 1),
    UART_MSR_EDGERING                   = (1 << 2),
    UART_MSR_DELTADETECT                = (1 << 3),
    UART_MSR_CLEAR                      = (1 << 4),
    UART_MSR_READY                      = (1 << 5),
    UART_MSR_RING                       = (1 << 6),
    UART_MSR_DETECT                     = (1 << 7)

};

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

    pic_set_routine(device, handle_irq);
    io_outb(uartDevice->port + UART_IER, UART_IER_NULL);
    io_outb(uartDevice->port + UART_LCR, UART_LCR_5BITS | UART_LCR_1STOP | UART_LCR_NOPARITY);
    io_outb(uartDevice->port + UART_THR, 0x03);
    io_outb(uartDevice->port + UART_IER, UART_IER_NULL);
    io_outb(uartDevice->port + UART_LCR, UART_LCR_8BITS | UART_LCR_1STOP | UART_LCR_NOPARITY);
    io_outb(uartDevice->port + UART_FCR, UART_FCR_ENABLE | UART_FCR_RECEIVE | UART_FCR_TRANSMIT | UART_FCR_SIZE3);
    io_outb(uartDevice->port + UART_MCR, UART_MCR_READY | UART_MCR_REQUEST | UART_MCR_AUX2);
    io_outb(uartDevice->port + UART_IER, UART_IER_RECEIVE);

}

static unsigned int check(struct base_device *device)
{

    return device->type == UART_DEVICE_TYPE;

}

static unsigned int read_terminal_data(struct terminal_interface *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct uart_driver *driver = (struct uart_driver *)self->driver;

    return circular_stream_read(&driver->stream, count, buffer);

}

static unsigned int write_terminal_data(struct terminal_interface *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct uart_driver *driver = (struct uart_driver *)self->driver;

    return circular_stream_write(&driver->stream, count, buffer);

}

void uart_init_driver(struct uart_driver *driver)
{

    memory_clear(driver, sizeof (struct uart_driver));
    base_init_driver(&driver->base, "uart", 0, check, attach);
    terminal_init_interface(&driver->iterminal, &driver->base, read_terminal_data, write_terminal_data);

}

