#include <fudge/module.h>
#include <x86/cpu.h>
#include <x86/idt.h>
#include <x86/gdt.h>
#include <base/base.h>
#include <arch/x86/io/io.h>
#include "pic.h"

#define PIC_ROUTINE_SLOTS               16

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

static struct pic_routine routines[PIC_ROUTINE_SLOTS];

static void raise(unsigned int index)
{

    if (!routines[index].callback)
        return;

    routines[index].callback(routines[index].device);

}

void pic_set_mask(unsigned short port, unsigned char mask)
{

    io_outb(port, mask);

}

void pic_enable_line(unsigned short port, unsigned char line)
{

    io_outb(port, io_inb(port) & ~(1 << (line % 8)));

}

void pic_disable_line(unsigned short port, unsigned char line)
{

    io_outb(port, io_inb(port) | (1 << (line % 8)));

}

static unsigned char pic_get_status(unsigned short port, unsigned char type)
{

    io_outb(port, type);

    return io_inb(port);

}

unsigned char pic_get_raised_lines(unsigned short port)
{

    return pic_get_status(port, PIC_COMMAND_IRR);

}

unsigned char pic_get_serviced_lines(unsigned short port)
{

    return pic_get_status(port, PIC_COMMAND_ISR);

}

static void reset(unsigned int slave)
{

    if (slave)
        io_outb(PIC_REGISTER_COMMAND1, PIC_COMMAND_EOI);

    io_outb(PIC_REGISTER_COMMAND0, PIC_COMMAND_EOI);

}

unsigned short pic_interrupt(struct pic_registers *registers)
{

    raise(registers->index);
    reset(registers->slave);

    return registers->interrupt.data;

}

unsigned int pic_set_routine(unsigned int index, struct base_device *device, void (*callback)(struct base_device *device))
{

    if (index > PIC_ROUTINE_SLOTS)
        return 0;

    if (routines[index].device)
        return 0;

    routines[index].device = device;
    routines[index].callback = callback;

    if (index >= 8)
        pic_enable_line(PIC_REGISTER_DATA1, index);
    else
        pic_enable_line(PIC_REGISTER_DATA0, index);

    return 1;

}

unsigned int pic_unset_routine(unsigned int index, struct base_device *device)
{

    if (index > PIC_ROUTINE_SLOTS)
        return 0;

    if (routines[index].device != device)
        return 0;

    routines[index].device = 0;
    routines[index].callback = 0;

    if (index >= 8)
        pic_disable_line(PIC_REGISTER_DATA1, index);
    else
        pic_disable_line(PIC_REGISTER_DATA0, index);

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

    struct idt_pointer *idtp = cpu_get_idt();
    unsigned int offset = sizeof (struct gdt_entry) * GDT_INDEX_KCODE;

    memory_clear(&routines, sizeof (struct pic_routine) * PIC_ROUTINE_SLOTS);
    setup_chip(PIC_REGISTER_COMMAND0, PIC_REGISTER_DATA0, PIC_DATA_VECTOR0, 0x04);
    setup_chip(PIC_REGISTER_COMMAND1, PIC_REGISTER_DATA1, PIC_DATA_VECTOR1, 0x02);
    idt_set_entry(idtp, PIC_DATA_VECTOR0 + 0x00, pic_routine00, offset, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(idtp, PIC_DATA_VECTOR0 + 0x01, pic_routine01, offset, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(idtp, PIC_DATA_VECTOR0 + 0x02, pic_routine02, offset, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(idtp, PIC_DATA_VECTOR0 + 0x03, pic_routine03, offset, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(idtp, PIC_DATA_VECTOR0 + 0x04, pic_routine04, offset, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(idtp, PIC_DATA_VECTOR0 + 0x05, pic_routine05, offset, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(idtp, PIC_DATA_VECTOR0 + 0x06, pic_routine06, offset, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(idtp, PIC_DATA_VECTOR0 + 0x07, pic_routine07, offset, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(idtp, PIC_DATA_VECTOR1 + 0x00, pic_routine08, offset, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(idtp, PIC_DATA_VECTOR1 + 0x01, pic_routine09, offset, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(idtp, PIC_DATA_VECTOR1 + 0x02, pic_routine0A, offset, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(idtp, PIC_DATA_VECTOR1 + 0x03, pic_routine0B, offset, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(idtp, PIC_DATA_VECTOR1 + 0x04, pic_routine0C, offset, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(idtp, PIC_DATA_VECTOR1 + 0x05, pic_routine0D, offset, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(idtp, PIC_DATA_VECTOR1 + 0x06, pic_routine0E, offset, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(idtp, PIC_DATA_VECTOR1 + 0x07, pic_routine0F, offset, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    pic_set_mask(PIC_REGISTER_DATA0, 0xFF);
    pic_set_mask(PIC_REGISTER_DATA1, 0xFF);
    pic_enable_line(PIC_REGISTER_DATA0, 2);

}

void destroy()
{

}

