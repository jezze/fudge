#include <fudge/kernel.h>
#include "idt.h"

static struct idt_entry entries[IDT_ENTRY_SLOTS];
static struct idt_pointer pointer;

void idt_set_entry(struct idt_entry *entry, void (*callback)(), unsigned short selector, unsigned char flags)
{

    unsigned int base = (unsigned int)callback;

    entry->baseLow = (base & 0xFFFF);
    entry->baseHigh = (base >> 16);
    entry->selector = selector;
    entry->flags = flags;

}

struct idt_pointer *idt_setup_pointer()
{

    memory_clear(&entries, sizeof (struct idt_entry) * IDT_ENTRY_SLOTS);

    pointer.base = entries;
    pointer.limit = (sizeof (struct idt_entry) * IDT_ENTRY_SLOTS) - 1;

    return &pointer;

}

