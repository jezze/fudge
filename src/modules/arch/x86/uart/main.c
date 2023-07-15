#include <fudge.h>
#include <kernel.h>
#include <modules/base/driver.h>
#include <modules/system/system.h>
#include <modules/console/console.h>
#include <modules/arch/x86/platform/platform.h>
#include <modules/arch/x86/pic/pic.h>
#include <modules/arch/x86/apic/apic.h>
#include <modules/arch/x86/io/io.h>

#define REG_RBR                         0x0000
#define REG_THR                         0x0000
#define REG_DLL                         0x0000
#define REG_DLM                         0x0000
#define REG_IER                         0x0001
#define REG_IER_NULL                    0
#define REG_IER_RECEIVE                 (1 << 0)
#define REG_IER_TRANSMIT                (1 << 1)
#define REG_IER_LINE                    (1 << 2)
#define REG_IER_MODEM                   (1 << 3)
#define REG_IER_SLEEP                   (1 << 4)
#define REG_IER_POWER                   (1 << 5)
#define REG_IIR                         0x0002
#define REG_IIR_PENDING                 (1 << 0)
#define REG_IIR_MODEMSTATUS             (0 << 1)
#define REG_IIR_TRANSMIT                (1 << 1)
#define REG_IIR_RECEIVE                 (2 << 1)
#define REG_IIR_LINE                    (3 << 1)
#define REG_IIR_TIMEOUT                 (6 << 1)
#define REG_IIR_LARGE                   (1 << 5)
#define REG_IIR_NOFIFO                  (1 << 6)
#define REG_IIR_ERRORFIFO               (2 << 6)
#define REG_IIR_FIFO                    (3 << 6)
#define REG_FCR                         0x0002
#define REG_FCR_ENABLE                  (1 << 0)
#define REG_FCR_RECEIVE                 (1 << 1)
#define REG_FCR_TRANSMIT                (1 << 2)
#define REG_FCR_SELECT                  (1 << 3)
#define REG_FCR_LARGE                   (1 << 5)
#define REG_FCR_SIZE0                   (0 << 6)
#define REG_FCR_SIZE1                   (1 << 6)
#define REG_FCR_SIZE2                   (2 << 6)
#define REG_FCR_SIZE3                   (3 << 6)
#define REG_LCR                         0x0003
#define REG_LCR_5BITS                   (0 << 0)
#define REG_LCR_6BITS                   (1 << 0)
#define REG_LCR_7BITS                   (2 << 0)
#define REG_LCR_8BITS                   (3 << 0)
#define REG_LCR_1STOP                   (0 << 2)
#define REG_LCR_2STOP                   (1 << 2)
#define REG_LCR_NOPARITY                (0 << 3)
#define REG_LCR_ODDPARITY               (1 << 3)
#define REG_LCR_EVENPARITY              (3 << 3)
#define REG_LCR_MARKPARITY              (5 << 3)
#define REG_LCR_SPACEPARITY             (7 << 3)
#define REG_LCR_BREAK                   (1 << 6)
#define REG_LCR_LATCH                   (1 << 7)
#define REG_MCR                         0x0004
#define REG_MCR_READY                   (1 << 0)
#define REG_MCR_REQUEST                 (1 << 1)
#define REG_MCR_AUX1                    (1 << 2)
#define REG_MCR_AUX2                    (1 << 3)
#define REG_MCR_LOOPBACK                (1 << 4)
#define REG_MCR_AUTOFLOW                (1 << 5)
#define REG_LSR                         0x0005
#define REG_LSR_READY                   (1 << 0)
#define REG_LSR_OVERRUN                 (1 << 1)
#define REG_LSR_PARITY                  (1 << 2)
#define REG_LSR_FRAMING                 (1 << 3)
#define REG_LSR_BREAK                   (1 << 4)
#define REG_LSR_TRANSMIT                (1 << 5)
#define REG_LSR_RECEIVE                 (1 << 6)
#define REG_LSR_FIFO                    (1 << 7)
#define REG_MSR                         0x0006
#define REG_MSR_DELTACLEAR              (1 << 0)
#define REG_MSR_DELTAREADY              (1 << 1)
#define REG_MSR_EDGERING                (1 << 2)
#define REG_MSR_DELTADETECT             (1 << 3)
#define REG_MSR_CLEAR                   (1 << 4)
#define REG_MSR_READY                   (1 << 5)
#define REG_MSR_RING                    (1 << 6)
#define REG_MSR_DETECT                  (1 << 7)
#define REG_SCR                         0x0007
#define BAUDRATE_50                     0x0900
#define BAUDRATE_110                    0x0417
#define BAUDRATE_220                    0x020C
#define BAUDRATE_300                    0x0180
#define BAUDRATE_600                    0x00C0
#define BAUDRATE_1200                   0x0060
#define BAUDRATE_2400                   0x0030
#define BAUDRATE_4800                   0x0018
#define BAUDRATE_9600                   0x000C
#define BAUDRATE_19200                  0x0006
#define BAUDRATE_38400                  0x0003
#define BAUDRATE_57600                  0x0002
#define BAUDRATE_115200                 0x0001

static struct base_driver driver;
static struct console_interface consoleinterface;
static unsigned short io;

unsigned char uart_get(void)
{

    while (!(io_inb(io + REG_LSR) & REG_LSR_READY));

    return io_inb(io);

}

void uart_put(unsigned char c)
{

    while (!(io_inb(io + REG_LSR) & REG_LSR_TRANSMIT));

    io_outb(io, c);

}

static void handleirq(unsigned int irq)
{

    unsigned char data = uart_get();

    console_notifydata(&consoleinterface, data);

}

static unsigned int send(void *buffer, unsigned int count)
{

    unsigned char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (b[i] == '\n')
            uart_put('\r');

        uart_put(b[i]);

    }

    return count;

}

static unsigned int consoleinterface_notifydata(struct list *links, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    return (event == EVENT_DATA) ? send(data, count) : 0;

}

static void driver_init(unsigned int id)
{

    console_initinterface(&consoleinterface, id);

    consoleinterface.data.operations.notify = consoleinterface_notifydata;

}

static unsigned int driver_match(unsigned int id)
{

    return id == PLATFORM_UART1;

}

static void driver_reset(unsigned int id)
{

    io = platform_getbase(id);

    io_outb(io + REG_IER, REG_IER_NULL);
    io_outb(io + REG_LCR, REG_LCR_5BITS | REG_LCR_1STOP | REG_LCR_NOPARITY);
    io_outb(io + REG_THR, 0x03);
    io_outb(io + REG_IER, REG_IER_NULL);
    io_outb(io + REG_LCR, REG_LCR_8BITS | REG_LCR_1STOP | REG_LCR_NOPARITY);
    io_outb(io + REG_FCR, REG_FCR_ENABLE | REG_FCR_RECEIVE | REG_FCR_TRANSMIT | REG_FCR_SIZE3);
    io_outb(io + REG_MCR, REG_MCR_READY | REG_MCR_REQUEST | REG_MCR_AUX2);
    io_outb(io + REG_IER, REG_IER_RECEIVE);

}

static void driver_attach(unsigned int id)
{

    unsigned short irq = platform_getirq(id);

    console_registerinterface(&consoleinterface);
    pic_setroutine(irq, handleirq);
    apic_setroutine(irq, handleirq);

}

static void driver_detach(unsigned int id)
{

    unsigned short irq = platform_getirq(id);

    console_unregisterinterface(&consoleinterface);
    pic_unsetroutine(irq);
    apic_unsetroutine(irq);

}

void module_init(void)
{

    base_initdriver(&driver, "uart", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PLATFORM_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PLATFORM_BUS);

}

