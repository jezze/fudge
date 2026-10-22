#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/console.h>
#include "amiga.h"

#define REG_CUSTOM                      0x00DFF000
#define REG_SERDATR                     0x18
#define REG_SERDATR_TBE                 0x2000
#define REG_SERDAT                      0x30
#define REG_SERDAT_STOP                 0x0100
#define REG_SERPER                      0x32
#define REG_SERPER_115200               30

static struct base_driver driver;
static struct console_interface consoleinterface;
static volatile unsigned short *custom = (volatile unsigned short *)REG_CUSTOM;

/* the bus clears the receive interrupt after this */
static void handleirq(unsigned int irq)
{

    console_notifydata(&consoleinterface, custom[REG_SERDATR / 2] & 0xFF);

}

static unsigned int consoleinterface_send(void *buffer, unsigned int count)
{

    unsigned char *b = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        while (!(custom[REG_SERDATR / 2] & REG_SERDATR_TBE));

        custom[REG_SERDAT / 2] = REG_SERDAT_STOP | b[i];

    }

    return count;

}

static void driver_init(unsigned int id)
{

    console_initinterface(&consoleinterface, id, consoleinterface_send);

}

static unsigned int driver_match(unsigned int id)
{

    return id == AMIGA_SERIAL;

}

static void driver_reset(unsigned int id)
{

    arch_kmap(REG_CUSTOM, REG_CUSTOM, 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

}

/* the period counts paula's colour clock (3546895 Hz on PAL machines) */
static void driver_attach(unsigned int id)
{

    custom[REG_SERPER / 2] = REG_SERPER_115200;

    console_registerinterface(&consoleinterface);
    amiga_setroutine(AMIGA_IRQ_RBF, handleirq);

}

static void driver_detach(unsigned int id)
{

    console_unregisterinterface(&consoleinterface);
    amiga_unsetroutine(AMIGA_IRQ_RBF, handleirq);

}

void module_init(void)
{

    base_initdriver(&driver, "amiga-serial", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, AMIGA_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, AMIGA_BUS);

}

