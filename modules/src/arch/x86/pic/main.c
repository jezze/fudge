#include <memory.h>
#include <base/base.h>
#include <arch/x86/idt.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/io/io.h>

static struct pic_routine routines[PIC_ROUTINE_SLOTS];

static void raise(unsigned int index)
{

    if (!routines[index].callback)
        return;

    routines[index].callback(routines[index].device);

}

static void remap()
{

    io_outb(PIC_COMMAND0, PIC_COMMAND_CONFIG);
    io_outb(PIC_DATA0, PIC_DATA_VECTOR0);
    io_outb(PIC_DATA0, 0x04);
    io_outb(PIC_DATA0, PIC_DATA_8086);

    io_outb(PIC_COMMAND1, PIC_COMMAND_CONFIG);
    io_outb(PIC_DATA1, PIC_DATA_VECTOR1);
    io_outb(PIC_DATA1, 0x02);
    io_outb(PIC_DATA1, PIC_DATA_8086);

}

void pic_enable()
{

    io_outb(PIC_DATA0, 0x00);
    io_outb(PIC_DATA1, 0x00);

}

void pic_disable()
{

    io_outb(PIC_DATA0, 0xFF);
    io_outb(PIC_DATA1, 0xFF);

}

static void reset(unsigned int slave)
{

    if (slave)
        io_outb(PIC_COMMAND1, PIC_COMMAND_EOI);

    io_outb(PIC_COMMAND0, PIC_COMMAND_EOI);

}

void pic_interrupt(struct pic_registers *registers)
{

    raise(registers->index);
    reset(registers->slave);

}

unsigned int pic_set_routine(unsigned int index, struct base_device *device, void (*callback)(struct base_device *device))
{

    if (routines[index].device)
        return 0;

    routines[index].device = device;
    routines[index].callback = callback;

    return 1;

}

unsigned int pic_unset_routine(unsigned int index, struct base_device *device)
{

    if (routines[index].device != device)
        return 0;

    routines[index].device = 0;
    routines[index].callback = 0;

    return 1;

}

void init()
{

    idt_set_entry(0x20, pic_routine00, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x21, pic_routine01, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x22, pic_routine02, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x23, pic_routine03, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x24, pic_routine04, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x25, pic_routine05, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x26, pic_routine06, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x27, pic_routine07, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x28, pic_routine08, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x29, pic_routine09, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x2A, pic_routine0A, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x2B, pic_routine0B, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x2C, pic_routine0C, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x2D, pic_routine0D, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x2E, pic_routine0E, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);
    idt_set_entry(0x2F, pic_routine0F, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_TYPE32INT);

    remap();
    pic_enable();

}

void destroy()
{

}

