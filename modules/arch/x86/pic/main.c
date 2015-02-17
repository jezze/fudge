#include <fudge.h>
#include <kernel.h>
#include <kernel/kernel.h>
#include <arch/x86/cpu.h>
#include <arch/x86/arch.h>
#include <arch/x86/io/io.h>
#include "pic.h"

#define PIC_ROUTINES                    16

enum pic_register
{

    PIC_REGISTER_COMMAND0               = 0x0020,
    PIC_REGISTER_DATA0                  = 0x0021,
    PIC_REGISTER_COMMAND1               = 0x00A0,
    PIC_REGISTER_DATA1                  = 0x00A1

};

enum pic_command
{

    PIC_COMMAND_IRR                     = 0x0A,
    PIC_COMMAND_ISR                     = 0x0B,
    PIC_COMMAND_CONFIG                  = 0x11,
    PIC_COMMAND_EOI                     = 0x20

};

enum pic_data
{

    PIC_DATA_8086                       = 0x01,
    PIC_DATA_VECTOR0                    = 0x20,
    PIC_DATA_VECTOR1                    = 0x28

};

static struct pic_routine routines[PIC_ROUTINES];

static void setchip(unsigned char command, unsigned char data, unsigned char vector, unsigned char wire)
{

    io_outb(command, PIC_COMMAND_CONFIG);
    io_outb(data, vector);
    io_outb(data, wire);
    io_outb(data, PIC_DATA_8086);

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

unsigned short pic_interrupt(void *stack)
{

    struct {struct cpu_general general; unsigned int index; unsigned int slave; struct cpu_interrupt interrupt;} *registers = stack;

    if (!routines[registers->index].callback)
        return arch_schedule(&registers->general, &registers->interrupt);

    routines[registers->index].callback(registers->index, routines[registers->index].id);

    if (registers->slave)
        io_outb(PIC_REGISTER_COMMAND1, PIC_COMMAND_EOI);

    io_outb(PIC_REGISTER_COMMAND0, PIC_COMMAND_EOI);

    return arch_schedule(&registers->general, &registers->interrupt);

}

unsigned int pic_setroutine(unsigned int irq, unsigned int id, void (*callback)(unsigned int irq, unsigned int id))
{

    if (irq >= PIC_ROUTINES)
        return 0;

    if (routines[irq].id)
        return 0;

    routines[irq].id = id;
    routines[irq].callback = callback;

    if (irq >= 8)
        enableline(PIC_REGISTER_DATA1, irq);
    else
        enableline(PIC_REGISTER_DATA0, irq);

    return 1;

}

unsigned int pic_unsetroutine(unsigned int irq, unsigned int id)
{

    if (irq >= PIC_ROUTINES)
        return 0;

    if (routines[irq].id != id)
        return 0;

    routines[irq].id = 0;
    routines[irq].callback = 0;

    if (irq >= 8)
        disableline(PIC_REGISTER_DATA1, irq);
    else
        disableline(PIC_REGISTER_DATA0, irq);

    return 1;

}

void module_init()
{

    setchip(PIC_REGISTER_COMMAND0, PIC_REGISTER_DATA0, PIC_DATA_VECTOR0, 0x04);
    setchip(PIC_REGISTER_COMMAND1, PIC_REGISTER_DATA1, PIC_DATA_VECTOR1, 0x02);
    setmask(PIC_REGISTER_DATA0, 0xFB);
    setmask(PIC_REGISTER_DATA1, 0xFF);

    while (getstatus(PIC_REGISTER_COMMAND1, PIC_COMMAND_ISR))
    {

        io_outb(PIC_REGISTER_COMMAND1, PIC_COMMAND_EOI);
        io_outb(PIC_REGISTER_COMMAND0, PIC_COMMAND_EOI);

    }

    while (getstatus(PIC_REGISTER_COMMAND0, PIC_COMMAND_ISR))
        io_outb(PIC_REGISTER_COMMAND0, PIC_COMMAND_EOI);

    arch_setinterrupt(PIC_DATA_VECTOR0 + 0x00, pic_routine00);
    arch_setinterrupt(PIC_DATA_VECTOR0 + 0x01, pic_routine01);
    arch_setinterrupt(PIC_DATA_VECTOR0 + 0x02, pic_routine02);
    arch_setinterrupt(PIC_DATA_VECTOR0 + 0x03, pic_routine03);
    arch_setinterrupt(PIC_DATA_VECTOR0 + 0x04, pic_routine04);
    arch_setinterrupt(PIC_DATA_VECTOR0 + 0x05, pic_routine05);
    arch_setinterrupt(PIC_DATA_VECTOR0 + 0x06, pic_routine06);
    arch_setinterrupt(PIC_DATA_VECTOR0 + 0x07, pic_routine07);
    arch_setinterrupt(PIC_DATA_VECTOR1 + 0x00, pic_routine08);
    arch_setinterrupt(PIC_DATA_VECTOR1 + 0x01, pic_routine09);
    arch_setinterrupt(PIC_DATA_VECTOR1 + 0x02, pic_routine0A);
    arch_setinterrupt(PIC_DATA_VECTOR1 + 0x03, pic_routine0B);
    arch_setinterrupt(PIC_DATA_VECTOR1 + 0x04, pic_routine0C);
    arch_setinterrupt(PIC_DATA_VECTOR1 + 0x05, pic_routine0D);
    arch_setinterrupt(PIC_DATA_VECTOR1 + 0x06, pic_routine0E);
    arch_setinterrupt(PIC_DATA_VECTOR1 + 0x07, pic_routine0F);

}

void module_register()
{

}

void module_unregister()
{

}

