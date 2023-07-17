#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/idt.h>
#include <kernel/x86/tss.h>
#include <kernel/x86/arch.h>
#include <modules/arch/x86/io/io.h>
#include "pic.h"

#define ROUTINES                        16
#define REG_COMMAND0                    0x0020
#define REG_COMMAND1                    0x00A0
#define REG_COMMAND_IRR                 0x0A
#define REG_COMMAND_ISR                 0x0B
#define REG_COMMAND_CONFIG              0x11
#define REG_COMMAND_EOI                 0x20
#define REG_DATA0                       0x0021
#define REG_DATA1                       0x00A1
#define REG_DATA_8086                   0x01
#define REG_DATA_VECTOR0                0x20
#define REG_DATA_VECTOR1                0x28

static struct arch_gdt *gdt = (struct arch_gdt *)ARCH_GDTPHYSICAL;
static struct arch_idt *idt = (struct arch_idt *)ARCH_IDTPHYSICAL;
static void (*routines[ROUTINES])(unsigned int irq);

static void setchip(unsigned char command, unsigned char data, unsigned char vector, unsigned char wire)
{

    io_outb(command, REG_COMMAND_CONFIG);
    io_outb(data, vector);
    io_outb(data, wire);
    io_outb(data, REG_DATA_8086);

}

static void setmask(unsigned short port, unsigned char mask)
{

    io_outb(port, mask);

}

static unsigned short getirqreg(unsigned char reg)
{

    io_outb(REG_COMMAND0, reg);
    io_outb(REG_COMMAND1, reg);

    return (io_inb(REG_COMMAND1) << 8) | io_inb(REG_COMMAND0);

}

static unsigned char getstatus(unsigned short port, unsigned char type)
{

    io_outb(port, type);

    return io_inb(port);

}

static void enableline(unsigned short port, unsigned char line)
{

    io_outb(port, io_inb(port) & ~(1 << (line % 8)));

}

static void disableline(unsigned short port, unsigned char line)
{

    io_outb(port, io_inb(port) | (1 << (line % 8)));

}

unsigned short pic_getirr(void)
{

    return getirqreg(REG_COMMAND_IRR);

}

unsigned short pic_getisr(void)
{

    return getirqreg(REG_COMMAND_ISR);

}

unsigned short pic_interrupt1(struct cpu_general general, unsigned int index, struct cpu_interrupt interrupt)
{

    if (index == 15)
    {

        if (getstatus(REG_COMMAND1, REG_COMMAND_ISR))
        {

            if (routines[index])
                routines[index](index);

            io_outb(REG_COMMAND1, REG_COMMAND_EOI);
            io_outb(REG_COMMAND0, REG_COMMAND_EOI);

        }

        else
        {

            io_outb(REG_COMMAND0, REG_COMMAND_EOI);

        }

    }

    else
    {

        if (routines[index])
            routines[index](index);

        io_outb(REG_COMMAND1, REG_COMMAND_EOI);
        io_outb(REG_COMMAND0, REG_COMMAND_EOI);

    }

    return arch_resume(&general, &interrupt);

}

unsigned short pic_interrupt0(struct cpu_general general, unsigned int index, struct cpu_interrupt interrupt)
{

    if (index == 7)
    {

        if (getstatus(REG_COMMAND0, REG_COMMAND_ISR))
        {

            if (routines[index])
                routines[index](index);

            io_outb(REG_COMMAND0, REG_COMMAND_EOI);

        }

    }

    else
    {

        if (routines[index])
            routines[index](index);

        io_outb(REG_COMMAND0, REG_COMMAND_EOI);

    }

    return arch_resume(&general, &interrupt);

}

unsigned int pic_setroutine(unsigned int irq, void (*routine)(unsigned int irq))
{

    if (irq >= ROUTINES)
        return 0;

    routines[irq] = routine;

    if (irq >= 8)
        enableline(REG_DATA1, irq);
    else
        enableline(REG_DATA0, irq);

    return 1;

}

unsigned int pic_unsetroutine(unsigned int irq)
{

    if (irq >= ROUTINES)
        return 0;

    routines[irq] = 0;

    if (irq >= 8)
        disableline(REG_DATA1, irq);
    else
        disableline(REG_DATA0, irq);

    return 1;

}

void pic_disable(void)
{

    setmask(REG_DATA0, 0xFF);
    setmask(REG_DATA1, 0xFF);

}

void module_init(void)
{

    unsigned int i;

    setchip(REG_COMMAND0, REG_DATA0, REG_DATA_VECTOR0, 0x04);
    setchip(REG_COMMAND1, REG_DATA1, REG_DATA_VECTOR1, 0x02);
    setmask(REG_DATA0, 0xFB);
    setmask(REG_DATA1, 0xFF);

    while (getstatus(REG_COMMAND1, REG_COMMAND_ISR))
    {

        io_outb(REG_COMMAND1, REG_COMMAND_EOI);
        io_outb(REG_COMMAND0, REG_COMMAND_EOI);

    }

    while (getstatus(REG_COMMAND0, REG_COMMAND_ISR))
        io_outb(REG_COMMAND0, REG_COMMAND_EOI);

    for (i = 0; i < 8; i++)
    {

        idt_setdescriptor(&idt->pointer, REG_DATA_VECTOR0 + i, (void (*)(void))((unsigned int)pic_isr0 + 8 * i), gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
        idt_setdescriptor(&idt->pointer, REG_DATA_VECTOR1 + i, (void (*)(void))((unsigned int)pic_isr1 + 8 * i), gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);

    }

}

