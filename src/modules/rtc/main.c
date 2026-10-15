#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include <modules/clock.h>
#include <modules/platform/platform.h>
#include <modules/pic/pic.h>
#include <modules/apic/apic.h>
#include <modules/io/io.h>

#define REG_COMMAND                     0x0000
#define REG_COMMAND_SECONDS             0x00
#define REG_COMMAND_MINUTES             0x02
#define REG_COMMAND_HOURS               0x04
#define REG_COMMAND_WEEKDAY             0x06
#define REG_COMMAND_DAY                 0x07
#define REG_COMMAND_MONTH               0x08
#define REG_COMMAND_YEAR                0x09
#define REG_DATA                        0x0001

static struct base_driver driver;
static struct clock_interface clockinterface;
static unsigned short io;

static unsigned char convert(unsigned char num)
{

    return ((num & 0xF0) >> 1) + ((num & 0xF0) >> 3) + (num & 0x0F);

}

static unsigned char read(unsigned int type)
{

    io_outb(io + REG_COMMAND, type);

    return convert(io_inb(io + REG_DATA));

}

static void handleirq(unsigned int irq)
{

}

static unsigned int clockinterface_getseconds(void)
{

    return read(REG_COMMAND_SECONDS);

}

static unsigned int clockinterface_getminutes(void)
{

    return read(REG_COMMAND_MINUTES);

}

static unsigned int clockinterface_gethours(void)
{

    return read(REG_COMMAND_HOURS);

}

static unsigned int clockinterface_getweekday(void)
{

    return read(REG_COMMAND_WEEKDAY);

}

static unsigned int clockinterface_getday(void)
{

    return read(REG_COMMAND_DAY);

}

static unsigned int clockinterface_getmonth(void)
{

    return read(REG_COMMAND_MONTH);

}

static unsigned int clockinterface_getyear(void)
{

    return 2000 + read(REG_COMMAND_YEAR);

}

static void driver_init(unsigned int id)
{

    clock_initinterface(&clockinterface, id, clockinterface_getseconds, clockinterface_getminutes, clockinterface_gethours, clockinterface_getweekday, clockinterface_getday, clockinterface_getmonth, clockinterface_getyear);

}

static unsigned int driver_match(unsigned int id)
{

    return id == PLATFORM_RTC;

}

static void driver_reset(unsigned int id)
{

    io = platform_getbase(id);

}

static void driver_attach(unsigned int id)
{

    unsigned short irq = platform_getirq(id);

    clock_registerinterface(&clockinterface);
    pic_setroutine(irq, handleirq);
    apic_setroutine(irq, handleirq);

}

static void driver_detach(unsigned int id)
{

    unsigned short irq = platform_getirq(id);

    clock_unregisterinterface(&clockinterface);
    pic_unsetroutine(irq, handleirq);
    apic_unsetroutine(irq, handleirq);

}

void module_init(void)
{

    base_initdriver(&driver, "rtc", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PLATFORM_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PLATFORM_BUS);

}

