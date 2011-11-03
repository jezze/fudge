#include <lib/memory.h>
#include <kernel/arch/x86/idt.h>

static struct idt_entry idt[IDT_TABLE_SLOTS];
static struct idt_pointer idtPointer;

void idt_set_gate(unsigned char index, void *callback, unsigned short selector, unsigned char flags)
{

    unsigned int base = (unsigned int)callback;

    idt[index].baseLow = (base & 0xFFFF);
    idt[index].baseHigh = (base >> 16) & 0xFFFF;
    idt[index].selector = selector;
    idt[index].zero = 0;
    idt[index].flags = flags | 0x60;

}

void idt_init()
{

    memory_set(&idt, 0, sizeof (struct idt_entry) * IDT_TABLE_SLOTS);

    idtPointer.base = idt;
    idtPointer.limit = (sizeof (struct idt_entry) * IDT_TABLE_SLOTS) - 1;
    idt_flush(&idtPointer);

}

