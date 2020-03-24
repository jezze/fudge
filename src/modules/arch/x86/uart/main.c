#include <fudge.h>
#include <kernel.h>
#include <modules/base/base.h>
#include <modules/system/system.h>
#include <modules/console/console.h>
#include <modules/arch/x86/platform/platform.h>
#include <modules/arch/x86/pic/pic.h>
#include <modules/arch/x86/io/io.h>

#define REGISTERRBR                     0x0000
#define REGISTERTHR                     0x0000
#define REGISTERDLL                     0x0000
#define REGISTERDLM                     0x0000
#define REGISTERIER                     0x0001
#define REGISTERIIR                     0x0002
#define REGISTERFCR                     0x0002
#define REGISTERLCR                     0x0003
#define REGISTERMCR                     0x0004
#define REGISTERLSR                     0x0005
#define REGISTERMSR                     0x0006
#define REGISTERSCR                     0x0007
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
#define IERNULL                         0
#define IERRECEIVE                      (1 << 0)
#define IERTRANSMIT                     (1 << 1)
#define IERLINE                         (1 << 2)
#define IERMODEM                        (1 << 3)
#define IERSLEEP                        (1 << 4)
#define IERPOWER                        (1 << 5)
#define IIRPENDING                      (1 << 0)
#define IIRMODEMSTATUS                  (0 << 1)
#define IIRTRANSMIT                     (1 << 1)
#define IIRRECEIVE                      (2 << 1)
#define IIRLINE                         (3 << 1)
#define IIRTIMEOUT                      (6 << 1)
#define IIRLARGE                        (1 << 5)
#define IIRNOFIFO                       (1 << 6)
#define IIRERRORFIFO                    (2 << 6)
#define IIRFIFO                         (3 << 6)
#define FCRENABLE                       (1 << 0)
#define FCRRECEIVE                      (1 << 1)
#define FCRTRANSMIT                     (1 << 2)
#define FCRSELECT                       (1 << 3)
#define FCRLARGE                        (1 << 5)
#define FCRSIZE0                        (0 << 6)
#define FCRSIZE1                        (1 << 6)
#define FCRSIZE2                        (2 << 6)
#define FCRSIZE3                        (3 << 6)
#define LCR5BITS                        (0 << 0)
#define LCR6BITS                        (1 << 0)
#define LCR7BITS                        (2 << 0)
#define LCR8BITS                        (3 << 0)
#define LCR1STOP                        (0 << 2)
#define LCR2STOP                        (1 << 2)
#define LCRNOPARITY                     (0 << 3)
#define LCRODDPARITY                    (1 << 3)
#define LCREVENPARITY                   (3 << 3)
#define LCRMARKPARITY                   (5 << 3)
#define LCRSPACEPARITY                  (7 << 3)
#define LCRBREAK                        (1 << 6)
#define LCRLATCH                        (1 << 7)
#define MCRREADY                        (1 << 0)
#define MCRREQUEST                      (1 << 1)
#define MCRAUX1                         (1 << 2)
#define MCRAUX2                         (1 << 3)
#define MCRLOOPBACK                     (1 << 4)
#define MCRAUTOFLOW                     (1 << 5)
#define LSRREADY                        (1 << 0)
#define LSROVERRUN                      (1 << 1)
#define LSRPARITY                       (1 << 2)
#define LSRFRAMING                      (1 << 3)
#define LSRBREAK                        (1 << 4)
#define LSRTRANSMIT                     (1 << 5)
#define LSRRECEIVE                      (1 << 6)
#define LSRFIFO                         (1 << 7)
#define MSRDELTACLEAR                   (1 << 0)
#define MSRDELTAREADY                   (1 << 1)
#define MSREDGERING                     (1 << 2)
#define MSRDELTADETECT                  (1 << 3)
#define MSRCLEAR                        (1 << 4)
#define MSRREADY                        (1 << 5)
#define MSRRING                         (1 << 6)
#define MSRDETECT                       (1 << 7)

static struct base_driver driver;
static struct console_interface consoleinterface;
static unsigned short io;

unsigned char uart_get(void)
{

    while (!(io_inb(io + REGISTERLSR) & LSRREADY));

    return io_inb(io);

}

void uart_put(unsigned char c)
{

    while (!(io_inb(io + REGISTERLSR) & LSRTRANSMIT));

    io_outb(io, c);

}

static void handleirq(unsigned int irq)
{

    unsigned char data = uart_get();

    console_notifydata(&consoleinterface, data);

}

static unsigned int consoleinterface_readctrl(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct ctrl_consolesettings settings;

    settings.scroll = 1;

    return memory_read(buffer, count, &settings, sizeof (struct ctrl_consolesettings), offset);

}

static unsigned int consoleinterface_writetransmit(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
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

    io_outb(io + REGISTERIER, IERNULL);
    io_outb(io + REGISTERLCR, LCR5BITS | LCR1STOP | LCRNOPARITY);
    io_outb(io + REGISTERTHR, 0x03);
    io_outb(io + REGISTERIER, IERNULL);
    io_outb(io + REGISTERLCR, LCR8BITS | LCR1STOP | LCRNOPARITY);
    io_outb(io + REGISTERFCR, FCRENABLE | FCRRECEIVE | FCRTRANSMIT | FCRSIZE3);
    io_outb(io + REGISTERMCR, MCRREADY | MCRREQUEST | MCRAUX2);
    io_outb(io + REGISTERIER, IERRECEIVE);

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

