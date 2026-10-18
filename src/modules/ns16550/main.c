#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/console.h>
#include <modules/virt/virt.h>

#define REG_DATA                        0x00
#define REG_IER                         0x01
#define REG_IER_RX                      (1 << 0)
#define REG_FCR                         0x02
#define REG_FCR_ENABLE                  0x07
#define REG_MCR                         0x04
#define REG_MCR_OUT2                    (1 << 3)
#define REG_LSR                         0x05
#define REG_LSR_RX                      (1 << 0)
#define REG_LSR_THRE                    (1 << 5)

static struct base_driver driver;
static struct console_interface consoleinterface;
static volatile unsigned char *uart;

static void handleirq(unsigned int irq)
{

    while (uart[REG_LSR] & REG_LSR_RX)
        console_notifydata(&consoleinterface, uart[REG_DATA]);

}

static unsigned int consoleinterface_send(void *buffer, unsigned int count)
{

    unsigned char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        while (!(uart[REG_LSR] & REG_LSR_THRE));

        uart[REG_DATA] = b[i];

    }

    return count;

}

static void driver_init(unsigned int id)
{

    console_initinterface(&consoleinterface, id, consoleinterface_send);

}

static unsigned int driver_match(unsigned int id)
{

    return id == VIRT_UART0;

}

static void driver_reset(unsigned int id)
{

    uart = (volatile unsigned char *)virt_getbase(id);

    arch_kmap(virt_getbase(id), virt_getbase(id), 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    uart[REG_FCR] = REG_FCR_ENABLE;
    uart[REG_MCR] = REG_MCR_OUT2;
    uart[REG_IER] = REG_IER_RX;

}

static void driver_attach(unsigned int id)
{

    console_registerinterface(&consoleinterface);
    virt_setroutine(virt_getirq(id), handleirq);

}

static void driver_detach(unsigned int id)
{

    console_unregisterinterface(&consoleinterface);
    virt_unsetroutine(virt_getirq(id), handleirq);

}

void module_init(void)
{

    base_initdriver(&driver, "ns16550", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, VIRT_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, VIRT_BUS);

}
