#include <lib/memory.h>
#include <kernel/arch/x86/cpu.h>
#include <kernel/arch/x86/idt.h>

static struct idt_entry entries[IDT_TABLE_SLOTS];
static struct idt_pointer pointer;

void idt_set_gate(unsigned char index, void *callback, unsigned short selector, unsigned char flags)
{

    unsigned int base = (unsigned int)callback;

    entries[index].baseLow = (base & 0xFFFF);
    entries[index].baseHigh = (base >> 16) & 0xFFFF;
    entries[index].selector = selector;
    entries[index].zero = 0;
    entries[index].flags = flags;

}

void idt_init()
{

    memory_set(&entries, 0, sizeof (struct idt_entry) * IDT_TABLE_SLOTS);

    pointer.base = entries;
    pointer.limit = (sizeof (struct idt_entry) * IDT_TABLE_SLOTS) - 1;

    cpu_set_idt(&pointer);

}

