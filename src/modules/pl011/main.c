#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/console.h>
#include <modules/icp/icp.h>

#define REG_DR                          0x00
#define REG_FR                          0x18
#define REG_FR_RXFE                     (1 << 4)
#define REG_FR_TXFF                     (1 << 5)
#define REG_IMSC                        0x38
#define REG_IMSC_RX                     (1 << 4)
#define REG_IMSC_RT                     (1 << 6)
#define REG_ICR                         0x44

static struct base_driver driver;
static struct console_interface consoleinterface;
static volatile unsigned int *uart;

static void handleirq(unsigned int irq)
{

    while (!(uart[REG_FR / 4] & REG_FR_RXFE))
        console_notifydata(&consoleinterface, uart[REG_DR / 4]);

    uart[REG_ICR / 4] = REG_IMSC_RX | REG_IMSC_RT;

}

static unsigned int consoleinterface_send(void *buffer, unsigned int count)
{

    unsigned char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        while (uart[REG_FR / 4] & REG_FR_TXFF);

        uart[REG_DR / 4] = b[i];

    }

    return count;

}

static void driver_init(unsigned int id)
{

    console_initinterface(&consoleinterface, id, consoleinterface_send);

}

static unsigned int driver_match(unsigned int id)
{

    return id == ICP_UART0;

}

static void driver_reset(unsigned int id)
{

    uart = (volatile unsigned int *)icp_getbase(id);

    arch_kmap(icp_getbase(id), icp_getbase(id), 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    uart[REG_IMSC / 4] = REG_IMSC_RX | REG_IMSC_RT;

}

static void driver_attach(unsigned int id)
{

    console_registerinterface(&consoleinterface);
    icp_setroutine(icp_getirq(id), handleirq);

}

static void driver_detach(unsigned int id)
{

    console_unregisterinterface(&consoleinterface);
    icp_unsetroutine(icp_getirq(id), handleirq);

}

void module_init(void)
{

    base_initdriver(&driver, "pl011", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, ICP_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, ICP_BUS);

}
