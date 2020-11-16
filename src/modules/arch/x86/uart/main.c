#include <fudge.h>
#include <kernel.h>
#include <modules/base/driver.h>
#include <modules/system/system.h>
#include <modules/console/console.h>
#include <modules/arch/x86/platform/platform.h>
#include <modules/arch/x86/pic/pic.h>
#include <modules/arch/x86/io/io.h>

#define R_RBR                           0x0000
#define R_THR                           0x0000
#define R_DLL                           0x0000
#define R_DLM                           0x0000
#define R_IER                           0x0001
#define R_IIR                           0x0002
#define R_FCR                           0x0002
#define R_LCR                           0x0003
#define R_MCR                           0x0004
#define R_LSR                           0x0005
#define R_MSR                           0x0006
#define R_SCR                           0x0007
#define F_IER_NULL                      0
#define F_IER_RECEIVE                   (1 << 0)
#define F_IER_TRANSMIT                  (1 << 1)
#define F_IER_LINE                      (1 << 2)
#define F_IER_MODEM                     (1 << 3)
#define F_IER_SLEEP                     (1 << 4)
#define F_IER_POWER                     (1 << 5)
#define F_IIR_PENDING                   (1 << 0)
#define F_IIR_MODEMSTATUS               (0 << 1)
#define F_IIR_TRANSMIT                  (1 << 1)
#define F_IIR_RECEIVE                   (2 << 1)
#define F_IIR_LINE                      (3 << 1)
#define F_IIR_TIMEOUT                   (6 << 1)
#define F_IIR_LARGE                     (1 << 5)
#define F_IIR_NOFIFO                    (1 << 6)
#define F_IIR_ERRORFIFO                 (2 << 6)
#define F_IIR_FIFO                      (3 << 6)
#define F_FCR_ENABLE                    (1 << 0)
#define F_FCR_RECEIVE                   (1 << 1)
#define F_FCR_TRANSMIT                  (1 << 2)
#define F_FCR_SELECT                    (1 << 3)
#define F_FCR_LARGE                     (1 << 5)
#define F_FCR_SIZE0                     (0 << 6)
#define F_FCR_SIZE1                     (1 << 6)
#define F_FCR_SIZE2                     (2 << 6)
#define F_FCR_SIZE3                     (3 << 6)
#define F_LCR_5BITS                     (0 << 0)
#define F_LCR_6BITS                     (1 << 0)
#define F_LCR_7BITS                     (2 << 0)
#define F_LCR_8BITS                     (3 << 0)
#define F_LCR_1STOP                     (0 << 2)
#define F_LCR_2STOP                     (1 << 2)
#define F_LCR_NOPARITY                  (0 << 3)
#define F_LCR_ODDPARITY                 (1 << 3)
#define F_LCR_EVENPARITY                (3 << 3)
#define F_LCR_MARKPARITY                (5 << 3)
#define F_LCR_SPACEPARITY               (7 << 3)
#define F_LCR_BREAK                     (1 << 6)
#define F_LCR_LATCH                     (1 << 7)
#define F_MCR_READY                     (1 << 0)
#define F_MCR_REQUEST                   (1 << 1)
#define F_MCR_AUX1                      (1 << 2)
#define F_MCR_AUX2                      (1 << 3)
#define F_MCR_LOOPBACK                  (1 << 4)
#define F_MCR_AUTOFLOW                  (1 << 5)
#define F_LSR_READY                     (1 << 0)
#define F_LSR_OVERRUN                   (1 << 1)
#define F_LSR_PARITY                    (1 << 2)
#define F_LSR_FRAMING                   (1 << 3)
#define F_LSR_BREAK                     (1 << 4)
#define F_LSR_TRANSMIT                  (1 << 5)
#define F_LSR_RECEIVE                   (1 << 6)
#define F_LSR_FIFO                      (1 << 7)
#define F_MSR_DELTACLEAR                (1 << 0)
#define F_MSR_DELTAREADY                (1 << 1)
#define F_MSR_EDGERING                  (1 << 2)
#define F_MSR_DELTADETECT               (1 << 3)
#define F_MSR_CLEAR                     (1 << 4)
#define F_MSR_READY                     (1 << 5)
#define F_MSR_RING                      (1 << 6)
#define F_MSR_DETECT                    (1 << 7)
#define BAUDRATE50                      0x0900
#define BAUDRATE110                     0x0417
#define BAUDRATE220                     0x020C
#define BAUDRATE300                     0x0180
#define BAUDRATE600                     0x00C0
#define BAUDRATE1200                    0x0060
#define BAUDRATE2400                    0x0030
#define BAUDRATE4800                    0x0018
#define BAUDRATE9600                    0x000C
#define BAUDRATE19200                   0x0006
#define BAUDRATE38400                   0x0003
#define BAUDRATE57600                   0x0002
#define BAUDRATE115200                  0x0001

static struct base_driver driver;
static struct console_interface consoleinterface;
static unsigned short io;

unsigned char uart_get(void)
{

    while (!(io_inb(io + R_LSR) & F_LSR_READY));

    return io_inb(io);

}

void uart_put(unsigned char c)
{

    while (!(io_inb(io + R_LSR) & F_LSR_TRANSMIT));

    io_outb(io, c);

}

static void handleirq(unsigned int irq)
{

    unsigned char data = uart_get();

    console_notifydata(&consoleinterface, data);

}

static unsigned int consoleinterface_readctrl(void *buffer, unsigned int count, unsigned int offset)
{

    struct ctrl_consolesettings settings;

    settings.scroll = 1;

    return memory_read(buffer, count, &settings, sizeof (struct ctrl_consolesettings), offset);

}

static unsigned int consoleinterface_writetransmit(void *buffer, unsigned int count, unsigned int offset)
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

static void driver_init(unsigned int id)
{

    console_initinterface(&consoleinterface, id);

    consoleinterface.ctrl.operations.read = consoleinterface_readctrl;
    consoleinterface.transmit.operations.write = consoleinterface_writetransmit;

}

static unsigned int driver_match(unsigned int id)
{

    return id == PLATFORM_UART1;

}

static void driver_reset(unsigned int id)
{

    io = platform_getbase(id);

    io_outb(io + R_IER, F_IER_NULL);
    io_outb(io + R_LCR, F_LCR_5BITS | F_LCR_1STOP | F_LCR_NOPARITY);
    io_outb(io + R_THR, 0x03);
    io_outb(io + R_IER, F_IER_NULL);
    io_outb(io + R_LCR, F_LCR_8BITS | F_LCR_1STOP | F_LCR_NOPARITY);
    io_outb(io + R_FCR, F_FCR_ENABLE | F_FCR_RECEIVE | F_FCR_TRANSMIT | F_FCR_SIZE3);
    io_outb(io + R_MCR, F_MCR_READY | F_MCR_REQUEST | F_MCR_AUX2);
    io_outb(io + R_IER, F_IER_RECEIVE);

}

static void driver_attach(unsigned int id)
{

    console_registerinterface(&consoleinterface);
    pic_setroutine(platform_getirq(id), handleirq);

}

static void driver_detach(unsigned int id)
{

    console_unregisterinterface(&consoleinterface);
    pic_unsetroutine(platform_getirq(id));

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

