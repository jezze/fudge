#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/timer.h>
#include "amiga.h"

#define INTERVAL                        (709379 / 60)
#define REG_CIAA                        0x00BFE000
#define REG_TALO                        0x401
#define REG_TAHI                        0x501
#define REG_CRA                         0xE01
#define REG_CRA_START                   0x01
#define REG_CRA_LOAD                    0x10

static struct base_driver driver;
static struct timer_interface timerinterface;
static volatile unsigned char *cia = (volatile unsigned char *)REG_CIAA;
static unsigned int jiffies;

static void handleirq(unsigned int irq)
{

    jiffies++;

    timer_notifytick(&timerinterface, jiffies);

}

static void driver_init(unsigned int id)
{

    timer_initinterface(&timerinterface, id, 60);

}

static unsigned int driver_match(unsigned int id)
{

    return id == AMIGA_TIMER;

}

static void driver_reset(unsigned int id)
{

    arch_kmap(REG_CIAA, REG_CIAA, 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

}

/* cia-a's timer a counts the e clock (709379 Hz on PAL machines) down in continuous mode */
static void driver_attach(unsigned int id)
{

    timer_registerinterface(&timerinterface);

    cia[REG_CRA] = 0;
    cia[REG_TALO] = INTERVAL & 0xFF;
    cia[REG_TAHI] = INTERVAL >> 8;
    cia[REG_CRA] = REG_CRA_START | REG_CRA_LOAD;

    amiga_setciaroutine(AMIGA_CIA_TA, handleirq);

}

static void driver_detach(unsigned int id)
{

    cia[REG_CRA] = 0;

    timer_unregisterinterface(&timerinterface);
    amiga_unsetciaroutine(AMIGA_CIA_TA, handleirq);

}

void module_init(void)
{

    base_initdriver(&driver, "amiga-timer", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, AMIGA_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, AMIGA_BUS);

}

