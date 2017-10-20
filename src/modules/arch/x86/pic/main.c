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
#define REGISTERCOMMAND0                0x0020
#define REGISTERDATA0                   0x0021
#define REGISTERCOMMAND1                0x00A0
#define REGISTERDATA1                   0x00A1
#define COMMANDIRR                      0x0A
#define COMMANDISR                      0x0B
#define COMMANDCONFIG                   0x11
#define COMMANDEOI                      0x20
#define DATA8086                        0x01
#define DATAVECTOR0                     0x20
#define DATAVECTOR1                     0x28

static struct arch_gdt *gdt = (struct arch_gdt *)ARCH_GDTADDRESS;
static struct arch_idt *idt = (struct arch_idt *)ARCH_IDTADDRESS;
static void (*routines[ROUTINES])(unsigned int irq);

static void setchip(unsigned char command, unsigned char data, unsigned char vector, unsigned char wire)
{

    io_outb(command, COMMANDCONFIG);
    io_outb(data, vector);
    io_outb(data, wire);
    io_outb(data, DATA8086);

}

static void setmask(unsigned short port, unsigned char mask)
{

    io_outb(port, mask);

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

unsigned short pic_interrupt(struct cpu_general general, unsigned int index, unsigned int slave, struct cpu_interrupt interrupt)
{

    routines[index](index);

    if (slave)
        io_outb(REGISTERCOMMAND1, COMMANDEOI);

    io_outb(REGISTERCOMMAND0, COMMANDEOI);

    return arch_resume(&general, &interrupt);

}

unsigned int pic_setroutine(unsigned int irq, void (*routine)(unsigned int irq))
{

    if (irq >= ROUTINES)
        return 0;

    routines[irq] = routine;

    if (irq >= 8)
        enableline(REGISTERDATA1, irq);
    else
        enableline(REGISTERDATA0, irq);

    return 1;

}

unsigned int pic_unsetroutine(unsigned int irq)
{

    if (irq >= ROUTINES)
        return 0;

    routines[irq] = 0;

    if (irq >= 8)
        disableline(REGISTERDATA1, irq);
    else
        disableline(REGISTERDATA0, irq);

    return 1;

}

void pic_disable(void)
{

    setmask(REGISTERDATA0, 0xFF);
    setmask(REGISTERDATA1, 0xFF);

}

void module_init(void)
{

    setchip(REGISTERCOMMAND0, REGISTERDATA0, DATAVECTOR0, 0x04);
    setchip(REGISTERCOMMAND1, REGISTERDATA1, DATAVECTOR1, 0x02);
    setmask(REGISTERDATA0, 0xFB);
    setmask(REGISTERDATA1, 0xFF);

    while (getstatus(REGISTERCOMMAND1, COMMANDISR))
    {

        io_outb(REGISTERCOMMAND1, COMMANDEOI);
        io_outb(REGISTERCOMMAND0, COMMANDEOI);

    }

    while (getstatus(REGISTERCOMMAND0, COMMANDISR))
        io_outb(REGISTERCOMMAND0, COMMANDEOI);

    idt_setdescriptor(&idt->pointer, DATAVECTOR0 + 0x00, pic_routine00, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, DATAVECTOR0 + 0x01, pic_routine01, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, DATAVECTOR0 + 0x02, pic_routine02, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, DATAVECTOR0 + 0x03, pic_routine03, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, DATAVECTOR0 + 0x04, pic_routine04, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, DATAVECTOR0 + 0x05, pic_routine05, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, DATAVECTOR0 + 0x06, pic_routine06, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, DATAVECTOR0 + 0x07, pic_routine07, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, DATAVECTOR1 + 0x00, pic_routine08, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, DATAVECTOR1 + 0x01, pic_routine09, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, DATAVECTOR1 + 0x02, pic_routine0A, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, DATAVECTOR1 + 0x03, pic_routine0B, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, DATAVECTOR1 + 0x04, pic_routine0C, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, DATAVECTOR1 + 0x05, pic_routine0D, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, DATAVECTOR1 + 0x06, pic_routine0E, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);
    idt_setdescriptor(&idt->pointer, DATAVECTOR1 + 0x07, pic_routine0F, gdt_getselector(&gdt->pointer, ARCH_KCODE), IDT_FLAG_PRESENT | IDT_FLAG_TYPE32INT);

}

