#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/timer.h>
#include <modules/platform/platform.h>
#include "../virt.h"

static struct base_driver driver;
static struct timer_interface timerinterface;
static unsigned int jiffies;

static void handleirq(unsigned int irq)
{

    cpu_settimer(cpu_gettimerfrequency() / 60);

    jiffies++;

    timer_notifytick(&timerinterface, jiffies);

}

static void driver_init(unsigned int id)
{

    timer_initinterface(&timerinterface, 60);

}

static unsigned int driver_match(unsigned int type, unsigned int id)
{

    return type == PLATFORM_BUS && id == VIRT_TIMER;

}

static void driver_reset(unsigned int id)
{

}

static void driver_attach(unsigned int id)
{

    timer_registerinterface(&timerinterface);
    cpu_settimer(cpu_gettimerfrequency() / 60);
    platform_setroutine(platform_getirq(id), handleirq);

}

static void driver_detach(unsigned int id)
{

    timer_unregisterinterface(&timerinterface);
    platform_unsetroutine(platform_getirq(id), handleirq);
    cpu_stoptimer();

}

void module_init(void)
{

    base_initdriver(&driver, "virt-timer", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver);

}

