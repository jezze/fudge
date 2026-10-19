#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/timer.h>
#include <modules/acpi/acpi.h>
#include <modules/pic/pic.h>
#include <modules/apic/apic.h>

#define HZ                              60
#define IRQ                             0x00
#define PERIODDIGITS                    6
#define REG_CAP                         0x0000
#define REG_CAP_LEGACY                  (1 << 15)
#define REG_PERIOD                      0x0004
#define REG_CONFIG                      0x0010
#define REG_CONFIG_ENABLE               (1 << 0)
#define REG_CONFIG_LEGACY               (1 << 1)
#define REG_COUNTERLOW                  0x00F0
#define REG_COUNTERHIGH                 0x00F4
#define REG_TIMER0CONFIG                0x0100
#define REG_TIMER0CONFIG_LEVEL          (1 << 1)
#define REG_TIMER0CONFIG_ENABLE         (1 << 2)
#define REG_TIMER0CONFIG_PERIODIC       (1 << 3)
#define REG_TIMER0CONFIG_PERIODICCAP    (1 << 4)
#define REG_TIMER0CONFIG_SETVALUE       (1 << 6)
#define REG_TIMER0CONFIG_32BIT          (1 << 8)
#define REG_TIMER0COMPARATOR            0x0108

static struct timer_interface timerinterface;
static unsigned long mmio;
static unsigned int jiffies;

static unsigned int read(unsigned int reg)
{

    return *(volatile unsigned int *)(mmio + reg);

}

static void write(unsigned int reg, unsigned int value)
{

    *(volatile unsigned int *)(mmio + reg) = value;

}

/* counter ticks per 1/HZ second: 10^15 / HZ / period in femtoseconds, one decimal digit at a time to stay within 32 bits */
static unsigned int getticks(unsigned int femtoseconds)
{

    unsigned int nanoseconds = 1000000000 / HZ;
    unsigned int ticks = nanoseconds / femtoseconds;
    unsigned int remainder = nanoseconds % femtoseconds;
    unsigned int i;

    for (i = 0; i < PERIODDIGITS; i++)
    {

        ticks = ticks * 10 + (remainder * 10) / femtoseconds;
        remainder = (remainder * 10) % femtoseconds;

    }

    return ticks;

}

static void handleirq(unsigned int irq)
{

    jiffies++;

    timer_notifytick(&timerinterface, jiffies);

}

static unsigned long findmmio(void)
{

    struct acpi_hpet *hpet = (struct acpi_hpet *)acpi_findheader("HPET");

    if (!hpet || hpet->addressspace != 0)
        return 0;

    if (hpet->address[4] || hpet->address[5] || hpet->address[6] || hpet->address[7])
        return 0;

    return (hpet->address[0] << 0) | (hpet->address[1] << 8) | (hpet->address[2] << 16) | ((unsigned long)hpet->address[3] << 24);

}

static unsigned int setup(void)
{

    unsigned int femtoseconds;
    unsigned int ticks;
    unsigned int config;

    mmio = findmmio();

    if (!mmio)
        return 0;

    arch_kmap(mmio, mmio, 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    femtoseconds = read(REG_PERIOD);

    if (!femtoseconds || femtoseconds > 100000000)
        return 0;

    if (!(read(REG_CAP) & REG_CAP_LEGACY) || !(read(REG_TIMER0CONFIG) & REG_TIMER0CONFIG_PERIODICCAP))
        return 0;

    ticks = getticks(femtoseconds);

    write(REG_CONFIG, read(REG_CONFIG) & ~(REG_CONFIG_ENABLE | REG_CONFIG_LEGACY));
    write(REG_COUNTERLOW, 0);
    write(REG_COUNTERHIGH, 0);

    config = read(REG_TIMER0CONFIG) & ~REG_TIMER0CONFIG_LEVEL;

    write(REG_TIMER0CONFIG, config | REG_TIMER0CONFIG_ENABLE | REG_TIMER0CONFIG_PERIODIC | REG_TIMER0CONFIG_SETVALUE | REG_TIMER0CONFIG_32BIT);
    write(REG_TIMER0COMPARATOR, ticks);
    write(REG_TIMER0COMPARATOR, ticks);
    write(REG_CONFIG, read(REG_CONFIG) | REG_CONFIG_ENABLE | REG_CONFIG_LEGACY);

    return 1;

}

void module_init(void)
{

    if (setup())
        timer_initinterface(&timerinterface, 0);
    else
        mmio = 0;

}

void module_register(void)
{

    if (!mmio)
        return;

    timer_registerinterface(&timerinterface);
    pic_setroutine(IRQ, handleirq);
    apic_setroutine(IRQ, handleirq);

}

void module_unregister(void)
{

    if (!mmio)
        return;

    timer_unregisterinterface(&timerinterface);
    pic_unsetroutine(IRQ, handleirq);
    apic_unsetroutine(IRQ, handleirq);
    write(REG_CONFIG, read(REG_CONFIG) & ~(REG_CONFIG_ENABLE | REG_CONFIG_LEGACY));

}

