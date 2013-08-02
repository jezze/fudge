#include <fudge/module.h>
#include <x86/cpu.h>
#include <x86/idt.h>
#include <x86/gdt.h>
#include <base/base.h>
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

static void pic_set_mask(unsigned short port, unsigned char mask)
{

    io_outb(port, mask);

}

static unsigned char pic_get_status(unsigned short port, unsigned char type)
{

    io_outb(port, type);

    return io_inb(port);

}

void pic_enable_line(unsigned short port, unsigned char line)
{

    io_outb(port, io_inb(port) & ~(1 << (line % 8)));

}

void pic_disable_line(unsigned short port, unsigned char line)
{

    io_outb(port, io_inb(port) | (1 << (line % 8)));

}

unsigned short pic_interrupt(void *stack)
{

    struct {struct cpu_general general; unsigned int index; unsigned int slave; struct cpu_interrupt interrupt;} *registers = stack;

    if (!routines[registers->index].callback)
        return registers->interrupt.data;

    routines[registers->index].callback(routines[registers->index].device);

    if (registers->slave)
        io_outb(PIC_REGISTER_COMMAND1, PIC_COMMAND_EOI);

    io_outb(PIC_REGISTER_COMMAND0, PIC_COMMAND_EOI);

    return registers->interrupt.data;

}

unsigned int pic_set_routine(struct base_device *device, void (*callback)(struct base_device *device))
{

    if (device->irq > PIC_ROUTINES)
        return 0;

    if (routines[device->irq].device)
        return 0;

    routines[device->irq].device = device;
    routines[device->irq].callback = callback;

    if (device->irq >= 8)
        pic_enable_line(PIC_REGISTER_DATA1, device->irq);
    else
        pic_enable_line(PIC_REGISTER_DATA0, device->irq);

    return 1;

}

unsigned int pic_unset_routine(struct base_device *device)
{

    if (device->irq > PIC_ROUTINES)
        return 0;

    if (routines[device->irq].device != device)
        return 0;

    routines[device->irq].device = 0;
    routines[device->irq].callback = 0;

    if (device->irq >= 8)
        pic_disable_line(PIC_REGISTER_DATA1, device->irq);
    else
        pic_disable_line(PIC_REGISTER_DATA0, device->irq);

    return 1;

}

void setup_chip(unsigned char command, unsigned char data, unsigned char vector, unsigned char wire)
{

    io_outb(command, PIC_COMMAND_CONFIG);
    io_outb(data, vector);
    io_outb(data, wire);
    io_outb(data, PIC_DATA_8086);

}

void init()
{

    struct idt_pointer *pointer = cpu_get_idt();
    unsigned short selector = sizeof (struct gdt_descriptor) * GDT_INDEX_KCODE;

    memory_clear(&routines, sizeof (struct pic_routine) * PIC_ROUTINES);
    setup_chip(PIC_REGISTER_COMMAND0, PIC_REGISTER_DATA0, PIC_DATA_VECTOR0, 0x04);
    setup_chip(PIC_REGISTER_COMMAND1, PIC_REGISTER_DATA1, PIC_DATA_VECTOR1, 0x02);
    idt_set_descriptor(pointer, PIC_DATA_VECTOR0 + 0x00, pic_routine00, selector, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(pointer, PIC_DATA_VECTOR0 + 0x01, pic_routine01, selector, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(pointer, PIC_DATA_VECTOR0 + 0x02, pic_routine02, selector, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(pointer, PIC_DATA_VECTOR0 + 0x03, pic_routine03, selector, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(pointer, PIC_DATA_VECTOR0 + 0x04, pic_routine04, selector, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(pointer, PIC_DATA_VECTOR0 + 0x05, pic_routine05, selector, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(pointer, PIC_DATA_VECTOR0 + 0x06, pic_routine06, selector, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(pointer, PIC_DATA_VECTOR0 + 0x07, pic_routine07, selector, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(pointer, PIC_DATA_VECTOR1 + 0x00, pic_routine08, selector, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(pointer, PIC_DATA_VECTOR1 + 0x01, pic_routine09, selector, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(pointer, PIC_DATA_VECTOR1 + 0x02, pic_routine0A, selector, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(pointer, PIC_DATA_VECTOR1 + 0x03, pic_routine0B, selector, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(pointer, PIC_DATA_VECTOR1 + 0x04, pic_routine0C, selector, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(pointer, PIC_DATA_VECTOR1 + 0x05, pic_routine0D, selector, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(pointer, PIC_DATA_VECTOR1 + 0x06, pic_routine0E, selector, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_descriptor(pointer, PIC_DATA_VECTOR1 + 0x07, pic_routine0F, selector, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    pic_set_mask(PIC_REGISTER_DATA0, 0xFB);
    pic_set_mask(PIC_REGISTER_DATA1, 0xFF);

    while (pic_get_status(PIC_REGISTER_COMMAND1, PIC_COMMAND_ISR))
    {

        io_outb(PIC_REGISTER_COMMAND1, PIC_COMMAND_EOI);
        io_outb(PIC_REGISTER_COMMAND0, PIC_COMMAND_EOI);

    }

    while (pic_get_status(PIC_REGISTER_COMMAND0, PIC_COMMAND_ISR))
        io_outb(PIC_REGISTER_COMMAND0, PIC_COMMAND_EOI);

}

void destroy()
{

}

