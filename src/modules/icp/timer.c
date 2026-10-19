#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/timer.h>
#include "icp.h"

#define FREQUENCY                       1000000
#define REG_LOAD                        0x00
#define REG_CONTROL                     0x08
#define REG_CONTROL_SIZE32              (1 << 1)
#define REG_CONTROL_INTERRUPT           (1 << 5)
#define REG_CONTROL_PERIODIC            (1 << 6)
#define REG_CONTROL_ENABLE              (1 << 7)
#define REG_INTCLR                      0x0C

static struct base_driver driver;
static struct timer_interface timerinterface;
static volatile unsigned int *timer;
static unsigned int jiffies;

static void handleirq(unsigned int irq)
{

    timer[REG_INTCLR / 4] = 1;

    jiffies++;

    timer_notifytick1(&timerinterface, jiffies);

    if (!(jiffies % 10))
        timer_notifytick10(&timerinterface, jiffies);

    if (!(jiffies % 100))
        timer_notifytick100(&timerinterface, jiffies);

    if (!(jiffies % 1000))
        timer_notifytick1000(&timerinterface, jiffies);

}

static void driver_init(unsigned int id)
{

    timer_initinterface(&timerinterface, id);

}

static unsigned int driver_match(unsigned int id)
{

    return id == ICP_TIMER1;

}

static void driver_reset(unsigned int id)
{

    timer = (volatile unsigned int *)icp_getbase(id);

    arch_kmap(icp_getbase(id), icp_getbase(id), 0x100, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    timer[REG_CONTROL / 4] = 0;
    timer[REG_LOAD / 4] = FREQUENCY / 60;
    timer[REG_INTCLR / 4] = 1;
    timer[REG_CONTROL / 4] = REG_CONTROL_SIZE32 | REG_CONTROL_INTERRUPT | REG_CONTROL_PERIODIC | REG_CONTROL_ENABLE;

}

static void driver_attach(unsigned int id)
{

    timer_registerinterface(&timerinterface);
    icp_setroutine(icp_getirq(id), handleirq);

}

static void driver_detach(unsigned int id)
{

    timer_unregisterinterface(&timerinterface);
    icp_unsetroutine(icp_getirq(id), handleirq);

}

void module_init(void)
{

    base_initdriver(&driver, "icp-timer", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, ICP_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, ICP_BUS);

}

