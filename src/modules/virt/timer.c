#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/timer.h>
#include "virt.h"

#define FREQUENCY                       10000000

static struct base_driver driver;
static struct timer_interface timerinterface;
static unsigned int jiffies;

/* the supervisor timer interrupt, rearmed through SBI for the next tick */
static void handleirq(void)
{

    cpu_settimer(cpu_gettime() + FREQUENCY / 60);

    jiffies++;

    timer_notifytick(&timerinterface, jiffies);

}

static void driver_init(unsigned int id)
{

    timer_initinterface(&timerinterface, id, 60);

}

static unsigned int driver_match(unsigned int id)
{

    return id == VIRT_TIMER;

}

static void driver_reset(unsigned int id)
{

}

static void driver_attach(unsigned int id)
{

    timer_registerinterface(&timerinterface);
    cpu_settimer(cpu_gettime() + FREQUENCY / 60);
    arch_setinterrupt(CPU_CAUSE_TIMER, handleirq);

}

static void driver_detach(unsigned int id)
{

    timer_unregisterinterface(&timerinterface);
    arch_setinterrupt(CPU_CAUSE_TIMER, 0);

}

void module_init(void)
{

    base_initdriver(&driver, "virt-timer", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, VIRT_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, VIRT_BUS);

}

