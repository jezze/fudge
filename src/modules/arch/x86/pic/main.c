#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/cpu.h>
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

    struct arch_context *context = arch_getcontext();

    routines[index](index);

    if (slave)
        io_outb(REGISTERCOMMAND1, COMMANDEOI);

    io_outb(REGISTERCOMMAND0, COMMANDEOI);

    return arch_resume(context, &general, &interrupt);

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

    arch_setinterrupt(DATAVECTOR0 + 0x00, pic_routine00);
    arch_setinterrupt(DATAVECTOR0 + 0x01, pic_routine01);
    arch_setinterrupt(DATAVECTOR0 + 0x02, pic_routine02);
    arch_setinterrupt(DATAVECTOR0 + 0x03, pic_routine03);
    arch_setinterrupt(DATAVECTOR0 + 0x04, pic_routine04);
    arch_setinterrupt(DATAVECTOR0 + 0x05, pic_routine05);
    arch_setinterrupt(DATAVECTOR0 + 0x06, pic_routine06);
    arch_setinterrupt(DATAVECTOR0 + 0x07, pic_routine07);
    arch_setinterrupt(DATAVECTOR1 + 0x00, pic_routine08);
    arch_setinterrupt(DATAVECTOR1 + 0x01, pic_routine09);
    arch_setinterrupt(DATAVECTOR1 + 0x02, pic_routine0A);
    arch_setinterrupt(DATAVECTOR1 + 0x03, pic_routine0B);
    arch_setinterrupt(DATAVECTOR1 + 0x04, pic_routine0C);
    arch_setinterrupt(DATAVECTOR1 + 0x05, pic_routine0D);
    arch_setinterrupt(DATAVECTOR1 + 0x06, pic_routine0E);
    arch_setinterrupt(DATAVECTOR1 + 0x07, pic_routine0F);

}

