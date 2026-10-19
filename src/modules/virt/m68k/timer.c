#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/timer.h>
#include "../virt.h"

#define INTERVAL                        (1000000000 / 60)
#define REG_TIMELOW                     0x00
#define REG_TIMEHIGH                    0x04
#define REG_ALARMLOW                    0x08
#define REG_ALARMHIGH                   0x0C
#define REG_IRQENABLED                  0x10
#define REG_CLEARINTERRUPT              0x1C

static struct base_driver driver;
static struct timer_interface timerinterface;
static volatile unsigned int *rtc;
static unsigned int jiffies;

/* the alarm is in nanoseconds; writing its low half arms it */
static void setalarm(void)
{

    unsigned int low = rtc[REG_TIMELOW / 4];
    unsigned int high = rtc[REG_TIMEHIGH / 4];
    unsigned int next = low + INTERVAL;

    rtc[REG_ALARMHIGH / 4] = (next < low) ? high + 1 : high;
    rtc[REG_ALARMLOW / 4] = next;

}

static void handleirq(unsigned int irq)
{

    rtc[REG_CLEARINTERRUPT / 4] = 1;

    setalarm();

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

    return id == VIRT_TIMER;

}

static void driver_reset(unsigned int id)
{

    rtc = (volatile unsigned int *)virt_getbase(id);

    arch_kmap(virt_getbase(id), virt_getbase(id), 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

}

static void driver_attach(unsigned int id)
{

    timer_registerinterface(&timerinterface);
    virt_setroutine(virt_getirq(id), handleirq);

    rtc[REG_IRQENABLED / 4] = 1;

    setalarm();

}

static void driver_detach(unsigned int id)
{

    rtc[REG_IRQENABLED / 4] = 0;

    timer_unregisterinterface(&timerinterface);
    virt_unsetroutine(virt_getirq(id), handleirq);

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

