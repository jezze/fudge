#include <module.h>
#include <kernel/resource.h>
#include <kernel/scheduler.h>
#include <system/system.h>
#include <base/base.h>
#include <base/terminal.h>
#include <arch/x86/platform/platform.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/io/io.h>

enum uart_register
{

    UART_REGISTER_RBR                   = 0x0000,
    UART_REGISTER_THR                   = 0x0000,
    UART_REGISTER_DLL                   = 0x0000,
    UART_REGISTER_DLM                   = 0x0000,
    UART_REGISTER_IER                   = 0x0001,
    UART_REGISTER_IIR                   = 0x0002,
    UART_REGISTER_FCR                   = 0x0002,
    UART_REGISTER_LCR                   = 0x0003,
    UART_REGISTER_MCR                   = 0x0004,
    UART_REGISTER_LSR                   = 0x0005,
    UART_REGISTER_MSR                   = 0x0006,
    UART_REGISTER_SCR                   = 0x0007

};

enum uart_baudrate
{

    UART_BAUDRATE_50                    = 0x0900,
    UART_BAUDRATE_110                   = 0x0417,
    UART_BAUDRATE_220                   = 0x020C,
    UART_BAUDRATE_300                   = 0x0180,
    UART_BAUDRATE_600                   = 0x00C0,
    UART_BAUDRATE_1200                  = 0x0060,
    UART_BAUDRATE_2400                  = 0x0030,
    UART_BAUDRATE_4800                  = 0x0018,
    UART_BAUDRATE_9600                  = 0x000C,
    UART_BAUDRATE_19200                 = 0x0006,
    UART_BAUDRATE_38400                 = 0x0003,
    UART_BAUDRATE_57600                 = 0x0002,
    UART_BAUDRATE_115200                = 0x0001

};

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

static struct base_driver driver;
static struct base_terminal_interface iterminal;
static struct base_terminal_node node;
static unsigned char buffer[512];
static struct buffer cfifo;
static struct scheduler_rendezvous rdata;
static unsigned short io;

static char read()
{

    while (!(io_inb(io + UART_REGISTER_LSR) & UART_LSR_READY));

    return io_inb(io);

}

static void write(char c)
{

    while (!(io_inb(io + UART_REGISTER_LSR) & UART_LSR_TRANSMIT));

    io_outb(io, c);

}

static void handleirq(unsigned int irq, struct base_bus *bus, unsigned int id)
{

    char data = read();
    unsigned int count = buffer_wcfifo(&cfifo, 1, &data);

    if (count)
        scheduler_rendezvous_unsleep(&rdata);

}

static unsigned int iterminal_rdata(unsigned int offset, unsigned int count, void *buffer)
{

    count = buffer_rcfifo(&cfifo, count, buffer);

    scheduler_rendezvous_sleep(&rdata, !count);

    return count;

}

static unsigned int iterminal_wdata(unsigned int offset, unsigned int count, void *buffer)
{

    unsigned char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
        write(b[i]);

    return count;

}

static unsigned int driver_match(struct base_bus *bus, unsigned int id)
{

    if (bus->type != PLATFORM_BUS_TYPE)
        return 0;

    return id == PLATFORM_UART1_DEVICE_TYPE;

}

static void driver_attach(struct base_bus *bus, unsigned int id)
{

    base_terminal_initinterface(&iterminal, bus, id, iterminal_rdata, iterminal_wdata);
    base_terminal_registerinterface(&iterminal);
    base_terminal_initnode(&node, &iterminal);
    base_terminal_registernode(&node);
    buffer_init(&cfifo, 1, 512, &buffer);
    pic_setroutine(bus, id, handleirq);

    io = platform_getbase(bus, id);

    io_outb(io + UART_REGISTER_IER, UART_IER_NULL);
    io_outb(io + UART_REGISTER_LCR, UART_LCR_5BITS | UART_LCR_1STOP | UART_LCR_NOPARITY);
    io_outb(io + UART_REGISTER_THR, 0x03);
    io_outb(io + UART_REGISTER_IER, UART_IER_NULL);
    io_outb(io + UART_REGISTER_LCR, UART_LCR_8BITS | UART_LCR_1STOP | UART_LCR_NOPARITY);
    io_outb(io + UART_REGISTER_FCR, UART_FCR_ENABLE | UART_FCR_RECEIVE | UART_FCR_TRANSMIT | UART_FCR_SIZE3);
    io_outb(io + UART_REGISTER_MCR, UART_MCR_READY | UART_MCR_REQUEST | UART_MCR_AUX2);
    io_outb(io + UART_REGISTER_IER, UART_IER_RECEIVE);

}

static void driver_detach(struct base_bus *bus, unsigned int id)
{

    pic_unsetroutine(bus, id);
    base_terminal_unregisterinterface(&iterminal);
    base_terminal_unregisternode(&node);

}

void init()
{

    base_initdriver(&driver, "uart", driver_match, driver_attach, driver_detach);
    base_registerdriver(&driver);

}

void destroy()
{

    base_unregisterdriver(&driver);

}

