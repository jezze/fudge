#include <fudge.h>
#include <kernel.h>
#include <modules/bus.h>
#include <modules/power.h>
#include <modules/io/io.h>
#include <modules/acpi/acpi.h>
#include "../platform.h"
#include "pc.h"

#define REG_PIT                         0x0040
#define REG_RTC                         0x0070
#define REG_UART4                       0x02E8
#define REG_UART2                       0x02F8
#define REG_UART3                       0x03E8
#define REG_UART1                       0x03F8
#define REG_DELAY                       0x0080
#define REG_PS2STATUS                   0x0064
#define REG_RESETCONTROL                0x0CF9
#define PS2_INPUTFULL                   0x02
#define PS2_RESET                       0xFE
#define RESETCONTROL_SYSTEM             0x02
#define RESETCONTROL_CPU                0x04
#define RESET_WAIT                      100000
#define IRQ_PIT                         0x00
#define IRQ_UART2                       0x03
#define IRQ_UART1                       0x04
#define IRQ_RTC                         0x08

struct device
{

    unsigned short base;
    unsigned short irq;

};

static struct base_bus bus;
static struct power_interface powerinterface;

static struct device devices[] = {
    {0},
    {REG_PIT, IRQ_PIT},
    {REG_RTC, IRQ_RTC},
    {REG_UART1, IRQ_UART1},
    {REG_UART2, IRQ_UART2},
    {REG_UART3, IRQ_UART1},
    {REG_UART4, IRQ_UART2}
};

unsigned long platform_getbase(unsigned int id)
{

    return devices[id].base;

}

unsigned int platform_getirq(unsigned int id)
{

    return devices[id].irq;

}

static void wait(void)
{

    unsigned int i;

    for (i = 0; i < RESET_WAIT; i++)
        io_inb(REG_DELAY);

}

static void resetps2(void)
{

    unsigned int i;

    for (i = 0; i < RESET_WAIT; i++)
    {

        if (!(io_inb(REG_PS2STATUS) & PS2_INPUTFULL))
            break;

    }

    io_outb(REG_PS2STATUS, PS2_RESET);
    wait();

}

static void resetcontrol(void)
{

    unsigned char value = io_inb(REG_RESETCONTROL) & ~(RESETCONTROL_SYSTEM | RESETCONTROL_CPU);

    io_outb(REG_RESETCONTROL, value | RESETCONTROL_SYSTEM);
    wait();
    io_outb(REG_RESETCONTROL, value | RESETCONTROL_SYSTEM | RESETCONTROL_CPU);
    wait();

}

static unsigned int poweroff(void)
{

    acpi_poweroff();

    return MESSAGE_FAILED;

}

static unsigned int reboot(void)
{

    acpi_reset();
    resetps2();
    resetcontrol();

    return MESSAGE_FAILED;

}

static void bus_setup(void)
{

}

static unsigned int bus_next(unsigned int id)
{

    return (id + 1) % 7;

}

void module_init(void)
{

    base_initbus(&bus, PLATFORM_BUS, "platform", bus_setup, bus_next);
    power_initinterface(&powerinterface, poweroff, reboot);

}

void module_register(void)
{

    base_registerbus(&bus);
    power_registerinterface(&powerinterface);

}

void module_unregister(void)
{

    power_unregisterinterface(&powerinterface);
    base_unregisterbus(&bus);

}

