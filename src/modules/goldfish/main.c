#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/clock.h>
#include <modules/virt/virt.h>

#define REG_TIMELOW                     0x00
#define REG_TIMEHIGH                    0x04

static struct base_driver driver;
static struct clock_interface clockinterface;
static volatile unsigned int *rtc;

/* nanoseconds since 1970, the low half read first latches the high half */
static unsigned long getseconds(void)
{

    unsigned long low = rtc[REG_TIMELOW / 4];
    unsigned long high = rtc[REG_TIMEHIGH / 4];

    return ((high << 32) | low) / 1000000000UL;

}

/* days since 1970 to year, month and day (Howard Hinnant's civil_from_days) */
static unsigned int getdate(unsigned int *month, unsigned int *day)
{

    unsigned int z = getseconds() / 86400 + 719468;
    unsigned int era = z / 146097;
    unsigned int doe = z - era * 146097;
    unsigned int yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    unsigned int doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    unsigned int mp = (5 * doy + 2) / 153;

    *day = doy - (153 * mp + 2) / 5 + 1;
    *month = (mp < 10) ? mp + 3 : mp - 9;

    return yoe + era * 400 + (*month <= 2);

}

static unsigned int clockinterface_getseconds(void)
{

    return getseconds() % 60;

}

static unsigned int clockinterface_getminutes(void)
{

    return (getseconds() / 60) % 60;

}

static unsigned int clockinterface_gethours(void)
{

    return (getseconds() / 3600) % 24;

}

static unsigned int clockinterface_getweekday(void)
{

    return (getseconds() / 86400 + 4) % 7;

}

static unsigned int clockinterface_getday(void)
{

    unsigned int month;
    unsigned int day;

    getdate(&month, &day);

    return day;

}

static unsigned int clockinterface_getmonth(void)
{

    unsigned int month;
    unsigned int day;

    getdate(&month, &day);

    return month;

}

static unsigned int clockinterface_getyear(void)
{

    unsigned int month;
    unsigned int day;

    return getdate(&month, &day);

}

static void driver_init(unsigned int id)
{

    clock_initinterface(&clockinterface, id, clockinterface_getseconds, clockinterface_getminutes, clockinterface_gethours, clockinterface_getweekday, clockinterface_getday, clockinterface_getmonth, clockinterface_getyear);

}

static unsigned int driver_match(unsigned int id)
{

    return id == VIRT_RTC;

}

static void driver_reset(unsigned int id)
{

    rtc = (volatile unsigned int *)virt_getbase(id);

    arch_kmap(virt_getbase(id), virt_getbase(id), 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

}

static void driver_attach(unsigned int id)
{

    clock_registerinterface(&clockinterface);

}

static void driver_detach(unsigned int id)
{

    clock_unregisterinterface(&clockinterface);

}

void module_init(void)
{

    base_initdriver(&driver, "goldfish", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, VIRT_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, VIRT_BUS);

}

