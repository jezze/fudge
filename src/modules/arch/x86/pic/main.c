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

unsigned short pic_interrupt(struct cpu_general general, unsigned int index, unsigned int slave, struct cpu_interrupt interrupt)
{

    routines[index](index);

    if (slave)
        io_outb(REG_COMMAND1, REG_COMMAND_EOI);

    io_outb(REG_COMMAND0, REG_COMMAND_EOI);

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

    idt_setdescriptor(&idt->pointer, REG_DATA_VECTOR0 + 0x00, pic_routine00, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, REG_DATA_VECTOR0 + 0x01, pic_routine01, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, REG_DATA_VECTOR0 + 0x02, pic_routine02, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, REG_DATA_VECTOR0 + 0x03, pic_routine03, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, REG_DATA_VECTOR0 + 0x04, pic_routine04, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, REG_DATA_VECTOR0 + 0x05, pic_routine05, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, REG_DATA_VECTOR0 + 0x06, pic_routine06, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, REG_DATA_VECTOR0 + 0x07, pic_routine07, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, REG_DATA_VECTOR1 + 0x00, pic_routine08, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, REG_DATA_VECTOR1 + 0x01, pic_routine09, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, REG_DATA_VECTOR1 + 0x02, pic_routine0A, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, REG_DATA_VECTOR1 + 0x03, pic_routine0B, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, REG_DATA_VECTOR1 + 0x04, pic_routine0C, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, REG_DATA_VECTOR1 + 0x05, pic_routine0D, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, REG_DATA_VECTOR1 + 0x06, pic_routine0E, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, REG_DATA_VECTOR1 + 0x07, pic_routine0F, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);

}

