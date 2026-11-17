#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/bus.h>
#include <modules/power.h>
#include <modules/platform/platform.h>
#include "../virt.h"

#define DEVICES                         (VIRT_FWCFG + 1)
#define ROUTINES                        96
#define SHARED                          4
#define REG_GICD                        0x08000000
#define REG_GICD_CTLR                   0x000
#define REG_GICD_ISENABLER              0x100
#define REG_GICD_ICENABLER              0x180
#define REG_GICD_IPRIORITYR             0x400
#define REG_GICD_ITARGETSR              0x800
#define REG_GICC                        0x08010000
#define REG_GICC_CTLR                   0x000
#define REG_GICC_PMR                    0x004
#define REG_GICC_IAR                    0x00C
#define REG_GICC_EOIR                   0x010
#define REG_RTC                         0x09010000
#define REG_UART0                       0x09000000
#define REG_FWCFG                       0x09020000
#define REG_VIRTIO                      0x0A000000
#define REG_VIRTIO_SIZE                 0x200
#define MAP_GICD                        0x0C000000
#define MAP_GICC                        0x0C010000
#define IRQ_TIMER                       27
#define IRQ_UART0                       33
#define IRQ_RTC                         34
#define IRQ_VIRTIO                      48
#define IRQ_SPURIOUS                    1020
#define VIRTIO_LAST                     31
#define PSCI_SYSTEMOFF                  0x84000008
#define PSCI_SYSTEMRESET                0x84000009

struct device
{

    unsigned long base;
    unsigned int irq;

};

static struct base_bus bus;
static struct power_interface powerinterface;
static void (*routines[ROUTINES][SHARED])(unsigned int irq);
static volatile unsigned int *gicd = (volatile unsigned int *)MAP_GICD;
static volatile unsigned char *gicdbytes = (volatile unsigned char *)MAP_GICD;
static volatile unsigned int *gicc = (volatile unsigned int *)MAP_GICC;

/* QEMU hands out the virtio-mmio transports from the last one down: the eight highest. The GIC is
   mapped away from its address, which shares a 2 MB region with task code at 0x08048000 */
static struct device devices[DEVICES] = {
    {0},
    {0, IRQ_TIMER},
    {REG_RTC, IRQ_RTC},
    {REG_UART0, IRQ_UART0},
    {REG_VIRTIO + (VIRTIO_LAST - 0) * REG_VIRTIO_SIZE, IRQ_VIRTIO + VIRTIO_LAST - 0},
    {REG_VIRTIO + (VIRTIO_LAST - 1) * REG_VIRTIO_SIZE, IRQ_VIRTIO + VIRTIO_LAST - 1},
    {REG_VIRTIO + (VIRTIO_LAST - 2) * REG_VIRTIO_SIZE, IRQ_VIRTIO + VIRTIO_LAST - 2},
    {REG_VIRTIO + (VIRTIO_LAST - 3) * REG_VIRTIO_SIZE, IRQ_VIRTIO + VIRTIO_LAST - 3},
    {REG_VIRTIO + (VIRTIO_LAST - 4) * REG_VIRTIO_SIZE, IRQ_VIRTIO + VIRTIO_LAST - 4},
    {REG_VIRTIO + (VIRTIO_LAST - 5) * REG_VIRTIO_SIZE, IRQ_VIRTIO + VIRTIO_LAST - 5},
    {REG_VIRTIO + (VIRTIO_LAST - 6) * REG_VIRTIO_SIZE, IRQ_VIRTIO + VIRTIO_LAST - 6},
    {REG_VIRTIO + (VIRTIO_LAST - 7) * REG_VIRTIO_SIZE, IRQ_VIRTIO + VIRTIO_LAST - 7},
    {REG_FWCFG, 0}
};

unsigned long platform_getbase(unsigned int id)
{

    return devices[id].base;

}

unsigned int platform_getirq(unsigned int id)
{

    return devices[id].irq;

}

static unsigned int hasroutines(unsigned int irq)
{

    unsigned int i;

    for (i = 0; i < SHARED; i++)
    {

        if (routines[irq][i])
            return 1;

    }

    return 0;

}

static void callroutines(unsigned int irq)
{

    unsigned int i;

    for (i = 0; i < SHARED; i++)
    {

        if (routines[irq][i])
            routines[irq][i](irq);

    }

}

static unsigned int addroutine(unsigned int irq, void (*routine)(unsigned int irq))
{

    unsigned int i;

    for (i = 0; i < SHARED; i++)
    {

        if (routines[irq][i] == routine)
            return 1;

    }

    for (i = 0; i < SHARED; i++)
    {

        if (!routines[irq][i])
        {

            routines[irq][i] = routine;

            return 1;

        }

    }

    return 0;

}

static void removeroutine(unsigned int irq, void (*routine)(unsigned int irq))
{

    unsigned int i;

    for (i = 0; i < SHARED; i++)
    {

        if (routines[irq][i] == routine)
            routines[irq][i] = 0;

    }

}

static void setenable(unsigned int irq, unsigned int enable)
{

    gicd[((enable) ? REG_GICD_ISENABLER : REG_GICD_ICENABLER) / 4 + irq / 32] = 1 << (irq % 32);

}

static void handleirq(void)
{

    unsigned int iar;

    while (((iar = gicc[REG_GICC_IAR / 4]) & 0x3FF) < IRQ_SPURIOUS)
    {

        unsigned int irq = iar & 0x3FF;

        if (irq < ROUTINES)
            callroutines(irq);

        gicc[REG_GICC_EOIR / 4] = iar;

    }

}

unsigned int platform_setroutine(unsigned int irq, void (*routine)(unsigned int irq))
{

    if (irq >= ROUTINES || !addroutine(irq, routine))
        return 0;

    gicdbytes[REG_GICD_IPRIORITYR + irq] = 0xA0;
    gicdbytes[REG_GICD_ITARGETSR + irq] = 0x01;
    setenable(irq, 1);

    return 1;

}

unsigned int platform_unsetroutine(unsigned int irq, void (*routine)(unsigned int irq))
{

    if (irq >= ROUTINES)
        return 0;

    removeroutine(irq, routine);

    if (!hasroutines(irq))
        setenable(irq, 0);

    return 1;

}

static unsigned int poweroff(void)
{

    cpu_psci(PSCI_SYSTEMOFF);

    return MESSAGE_FAILED;

}

static unsigned int reboot(void)
{

    cpu_psci(PSCI_SYSTEMRESET);

    return MESSAGE_FAILED;

}

static void bus_setup(void)
{

}

static unsigned int bus_next(unsigned int id)
{

    return (id + 1) % DEVICES;

}

void module_init(void)
{

    arch_kmap(REG_GICD, MAP_GICD, 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);
    arch_kmap(REG_GICC, MAP_GICC, 0x2000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    gicd[REG_GICD_CTLR / 4] = 1;
    gicc[REG_GICC_PMR / 4] = 0xFF;
    gicc[REG_GICC_CTLR / 4] = 1;

    arch_setinterrupt(CPU_INTERRUPT_IRQ, handleirq);
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

