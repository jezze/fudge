#include <lib/memory.h>
#include <kernel/arch/x86/idt.h>

static struct idt_entry idtEntries[IDT_TABLE_SLOTS];
static struct idt_pointer idtPointer;

void idt_set_gate(unsigned char index, void *callback, unsigned short selector, unsigned char flags)
{

    unsigned int base = (unsigned int)callback;

    idtEntries[index].baseLow = (base & 0xFFFF);
    idtEntries[index].baseHigh = (base >> 16) & 0xFFFF;
    idtEntries[index].selector = selector;
    idtEntries[index].zero = 0;
    idtEntries[index].flags = flags;

}

void idt_init()
{

    memory_set(&idtEntries, 0, sizeof (struct idt_entry) * IDT_TABLE_SLOTS);

    idtPointer.base = idtEntries;
    idtPointer.limit = (sizeof (struct idt_entry) * IDT_TABLE_SLOTS) - 1;
    idt_flush(&idtPointer);

}

